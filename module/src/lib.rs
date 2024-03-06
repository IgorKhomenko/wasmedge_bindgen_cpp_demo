pub mod funcs {
  #[allow(unused_imports)]
  use wasmedge_bindgen::*;
  use wasmedge_bindgen_macro::*;

  #[wasmedge_bindgen]
  pub fn say(name: String, age: i32, temp: f32) -> String {
    println!("name: {}", name);
    println!("age: {}", age);
    println!("temp: {}", temp);

    return format!("hello {}, age: {}, temp: {}", name.as_str(), age, temp);
  }
}