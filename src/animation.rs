/*
 * Copyright 2024 Andrey Kutejko <andy128k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * For more details see the file COPYING.
 */

use std::ops::Range;

pub trait Animation {
    fn frame(&self, frame: usize) -> Option<usize>;
    fn len(&self) -> usize;

    fn limit(self, limit: usize) -> impl Animation
    where
        Self: Sized,
    {
        Limit { inner: self, limit }
    }

    fn then<A: Animation>(self, another: A) -> impl Animation
    where
        Self: Sized,
    {
        Then {
            first: self,
            second: another,
        }
    }

    fn repeat(self, count: usize) -> impl Animation
    where
        Self: Sized,
    {
        Repeat { inner: self, count }
    }

    fn forever(self) -> impl InfiniteAnimation
    where
        Self: Sized,
    {
        Forever { inner: self }
    }
}

pub trait InfiniteAnimation {
    fn frame(&self, frame: usize) -> usize;
}

pub fn sequence(range: Range<usize>) -> impl Animation {
    Sequence {
        start: range.start,
        limit: range.len(),
        step: 1,
    }
}

pub fn rev_sequence(range: Range<usize>) -> impl Animation {
    let count = range.len();
    Sequence {
        start: range.start + count - 1,
        limit: count,
        step: -1,
    }
}

struct Sequence {
    start: usize,
    limit: usize,
    step: isize,
}

impl Animation for Sequence {
    fn frame(&self, frame: usize) -> Option<usize> {
        if frame >= self.limit {
            return None;
        }
        Some(
            self.start
                .saturating_add_signed((frame as isize) * self.step),
        )
    }

    fn len(&self) -> usize {
        self.limit
    }
}

struct Limit<A: Animation> {
    inner: A,
    limit: usize,
}

impl<A: Animation> Animation for Limit<A> {
    fn frame(&self, frame: usize) -> Option<usize> {
        if frame >= self.limit {
            return None;
        }
        self.inner.frame(frame)
    }

    fn len(&self) -> usize {
        self.inner.len().min(self.limit)
    }
}

pub fn bounce(range: Range<usize>) -> impl Animation {
    sequence(range.clone()).then(rev_sequence(range))
}

struct Then<F: Animation, S: Animation> {
    first: F,
    second: S,
}

impl<F: Animation, S: Animation> Animation for Then<F, S> {
    fn frame(&self, frame: usize) -> Option<usize> {
        let first_len = self.first.len();
        if frame < first_len {
            self.first.frame(frame)
        } else {
            self.second.frame(frame - first_len)
        }
    }

    fn len(&self) -> usize {
        self.first.len() + self.second.len()
    }
}

struct Repeat<A: Animation> {
    inner: A,
    count: usize,
}

impl<A: Animation> Animation for Repeat<A> {
    fn frame(&self, frame: usize) -> Option<usize> {
        let len = self.inner.len();
        if frame / len < self.count {
            self.inner.frame(frame % len)
        } else {
            None
        }
    }

    fn len(&self) -> usize {
        self.inner.len() * self.count
    }
}

struct Forever<A: Animation> {
    inner: A,
}

impl<A: Animation> InfiniteAnimation for Forever<A> {
    fn frame(&self, frame: usize) -> usize {
        self.inner.frame(frame % self.inner.len()).unwrap()
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::theme::PLAYER_ANIMATION_FRAMES;

    #[test]
    fn test_sequence() {
        let animation = sequence(0..3);

        assert_eq!(
            (0..4).map(|f| animation.frame(f)).collect::<Vec<_>>(),
            vec![Some(0), Some(1), Some(2), None,]
        );
    }

    #[test]
    fn test_repeat() {
        let animation = rev_sequence(4..6).repeat(3);

        assert_eq!(
            (0..7).map(|f| animation.frame(f)).collect::<Vec<_>>(),
            vec![Some(5), Some(4), Some(5), Some(4), Some(5), Some(4), None]
        );
    }

    #[test]
    fn test_player_animation() {
        let animation = sequence(PLAYER_ANIMATION_FRAMES)
            .limit(1)
            .repeat(5)
            .then(bounce(PLAYER_ANIMATION_FRAMES).repeat(2))
            .forever();

        assert_eq!(
            (0..26).map(|f| animation.frame(f)).collect::<Vec<_>>(),
            vec![0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0,]
        );
    }
}
