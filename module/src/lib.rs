pub mod funcs {
  // use wasmedge_bindgen_macro::*;
  use macroutils::*;

  use std::mem;

  #[wasmedge_bindgen]
  pub fn say(s: String, s2: String) -> String {
    println!("say s1={}", s);
    println!("say s2={}", s2);

    let r = String::from("hello ");
    return r + s.as_str();
  }

  #[no_mangle]
  pub unsafe extern fn allocate(size: i32) -> *const u8 {
    // println!("allocate size={}", size);

    let buffer = Vec::with_capacity(size as usize);

    let buffer = mem::ManuallyDrop::new(buffer);
    buffer.as_ptr() as *const u8
  }

  #[no_mangle]
  pub unsafe extern fn deallocate(pointer: *mut u8, size: i32) {
    // println!("deallocate size={}", size);

    drop(Vec::from_raw_parts(pointer, size as usize, size as usize));
  }
}