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

use std::{
    cell::RefCell,
    collections::HashMap,
    rc::Rc,
    sync::atomic::{AtomicU32, Ordering},
};

pub type Listener<T> = Rc<dyn Fn(&T)>;

pub struct Slot<T> {
    id: u32,
    listeners: RefCell<HashMap<u32, Listener<T>>>,
}

pub struct ListenerId(u32, u32);

impl<T> Slot<T> {
    pub fn new() -> Self {
        Self {
            id: new_id(),
            listeners: RefCell::new(HashMap::new()),
        }
    }

    pub fn connect(&self, listener: impl Fn(&T) + 'static) -> ListenerId {
        let listener_id = new_id();
        self.listeners
            .borrow_mut()
            .insert(listener_id, Rc::new(listener));
        ListenerId(self.id, listener_id)
    }

    pub fn disconnect(&self, listener_id: ListenerId) {
        if listener_id.0 == self.id {
            self.listeners.borrow_mut().remove(&listener_id.1);
        } else {
            eprintln!("WARN: Cannot disconnect a listener. Wrong slot id.");
        }
    }

    pub fn emit(&self, param: T) {
        for listener in self.listeners.borrow().values().cloned() {
            (listener)(&param);
        }
    }
}

fn new_id() -> u32 {
    static LAST_ID: AtomicU32 = AtomicU32::new(1);
    LAST_ID.fetch_add(1, Ordering::SeqCst)
}
