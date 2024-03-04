#include <wasmedge/wasmedge.h>
#include <stdio.h>
#include <iostream>
#include <vector>

const int exit_error_code = 1;
const int exit_success_code = 0;

const char *wasm_file_path = "../module/target/wasm32-wasi/release/funcs.wasm";

void splice(const unsigned char* array, int start, int end, unsigned char* spliced_array) {
  for (int i = start; i <= end; ++i) {
    *spliced_array++ = array[i];
  }
}

void printBytes(unsigned char *vec) {
  for (int i = 0; i < sizeof(int); ++i) {
    std::cout << "Byte: " << i << ": " << static_cast<int>(vec[i]) << std::endl;
  }
}

int allocate(WasmEdge_VMContext *VMCxt, int length) {
  WasmEdge_Value P[1], R[1]; 
  WasmEdge_String FuncName;
  WasmEdge_Result Res;

  // alloc a space for input args
  P[0] = WasmEdge_ValueGenI32(length);
  FuncName = WasmEdge_StringCreateByCString("allocate");
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 1, R, 1);
  WasmEdge_StringDelete(FuncName);

  if (WasmEdge_ResultOK(Res)) {
    return WasmEdge_ValueGetI32(R[0]);
  } else {
    return exit_error_code;
  }
}

int deallocate(WasmEdge_VMContext *VMCxt, int pointer, int size) {
  WasmEdge_Value P[2], R[0]; 
  WasmEdge_String FuncName;
  WasmEdge_Result Res;

  P[0] = WasmEdge_ValueGenI32(pointer);
  P[1] = WasmEdge_ValueGenI32(size);
  FuncName = WasmEdge_StringCreateByCString("deallocate");
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 2, R, 0);
  WasmEdge_StringDelete(FuncName);

  if (WasmEdge_ResultOK(Res)) {
    return exit_success_code;
  } else {
    return exit_error_code;
  }
}

// https://stackoverflow.com/questions/27687769/use-different-parameter-data-types-in-same-function-c
std::vector<int> settle(WasmEdge_VMContext *VMCxt, WasmEdge_MemoryInstanceContext *MemoryCxt, std::string input) {
  const char *cInput = input.c_str(); 

  int length_of_input = input.length();
  int pointer = allocate(VMCxt, length_of_input);

  WasmEdge_MemoryInstanceSetData(MemoryCxt, (unsigned char *)cInput, pointer, length_of_input);

  std::vector<int> res; 
  res.push_back(pointer);
  res.push_back(length_of_input);

  return res;
}

int parse_result(WasmEdge_VMContext *VMCxt, WasmEdge_MemoryInstanceContext *MemoryCxt, unsigned char *ret_pointer, unsigned char *ret_len) {
  int size = static_cast<int>(*ret_len);
  // int retPointer = static_cast<int>(*ret_pointer);

  int retPointer;
  std::memcpy(&retPointer, ret_pointer, sizeof(int));
  std::cout << std::hex << retPointer << '\n';

  // printf("retPointer: %d\n", retPointer);
  // printf("size: %d\n", size);

  unsigned char p_data[size];
  WasmEdge_MemoryInstanceGetData(MemoryCxt, p_data, retPointer, size * 3 * 4);
  deallocate(VMCxt, retPointer, size * 3 * 4);

  std::vector<int> p_values; 
  for (int i = 0; i < (size * 3); ++i) {
    // p_values.push_back(n);
  }

  	// for i in 0..size * 3 {
		// 	p_values[i] = i32::from_le_bytes(p_data[i*4..(i+1)*4].try_into().unwrap());
		// }

		// let mut results: Vec<Box<dyn Any + Send + Sync>> = Vec::with_capacity(size);


}

int main() {
  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt, WasmEdge_HostRegistration_Wasi);

  WasmEdge_StoreContext *StoreCxt = WasmEdge_StoreCreate();
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, StoreCxt);

  // add args/envs/preopens
  // this line start display logs from host functions 
  WasmEdge_ModuleInstanceContext *WasiCxt = WasmEdge_VMGetImportModuleContext(VMCxt, WasmEdge_HostRegistration_Wasi);
  WasmEdge_ModuleInstanceInitWASI(WasiCxt,  NULL, 0, NULL, 0,  NULL, 0);

  WasmEdge_Result Res;

  Res = WasmEdge_VMLoadWasmFromFile(VMCxt, wasm_file_path);
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM file loading failed\n");
    return exit_error_code;
  }
  Res = WasmEdge_VMValidate(VMCxt);
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM validation failed\n");
    return exit_error_code;
  }
  Res = WasmEdge_VMInstantiate(VMCxt);
  if (!WasmEdge_ResultOK(Res)) {
    printf("WASM instantiation failed\n");
    return exit_error_code;
  }



  std::vector<std::string> inputs; 
  inputs.push_back("bob");
  inputs.push_back("sam");
  //
  int inputs_count = inputs.size();
  //


  //
  // https://github.com/second-state/wasmedge-bindgen/blob/main/host/rust/src/lib.rs#L329
  //

  // alloc a space for input args
  int pointer_of_pointers = allocate(VMCxt, inputs_count * 4 * 2);
  if (pointer_of_pointers == exit_error_code) {
    printf("[allocate] Error\n");
    return exit_error_code;
  }

  // get active module
  const WasmEdge_ModuleInstanceContext *ActiveModuleCxt = WasmEdge_VMGetActiveModule(VMCxt);
  if (ActiveModuleCxt == NULL) {
    printf("WASM no active module found\n");
    return exit_error_code;
  }
  //
  // get active module's memory
  WasmEdge_String MemoryName = WasmEdge_StringCreateByCString("memory");
  WasmEdge_MemoryInstanceContext *MemoryCxt = WasmEdge_ModuleInstanceFindMemory(ActiveModuleCxt, MemoryName);
  WasmEdge_StringDelete(MemoryName);
  if (MemoryCxt == NULL) {
    printf("WASM no memory found\n");
    return exit_error_code;
  }


  int pos = 0;
  for (auto &inp : inputs) {
    // std::cout << "pos: " << pos << std::endl;

    std::vector<int> sr = settle(VMCxt, MemoryCxt, inp);

    int pointer = sr[0];
    //
    unsigned char *ucPointerLittleEndian = reinterpret_cast<unsigned char*>(&pointer);
    // printBytes(ucPointerLittleEndian);
    //
    WasmEdge_MemoryInstanceSetData(MemoryCxt,  ucPointerLittleEndian, pointer_of_pointers + pos * 4 * 2, sizeof(ucPointerLittleEndian));

    int length_of_input = sr[1];
    //
    unsigned char *ucLenghtOfInputLittleEndian = reinterpret_cast<unsigned char*>(&length_of_input);
    // printBytes(ucLenghtOfInputLittleEndian);
    WasmEdge_MemoryInstanceSetData(MemoryCxt, ucLenghtOfInputLittleEndian, pointer_of_pointers + pos * 4 * 2 + 4, sizeof(ucLenghtOfInputLittleEndian));

    ++pos;
  }


  // Run func
  WasmEdge_Value P[2], rets[1]; 
  WasmEdge_String FuncName;
  P[0] = WasmEdge_ValueGenI32(pointer_of_pointers); // params_pointer: *mut u32
  P[1] = WasmEdge_ValueGenI32(inputs_count); // params_count: i32
  FuncName = WasmEdge_StringCreateByCString("say");
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 2, rets, 1);
  WasmEdge_StringDelete(FuncName);
  if (WasmEdge_ResultOK(Res)) {
    printf("[say] Ok: %d\n", WasmEdge_ValueGetI32(rets[0]));
  } else {
    printf("[say] Error\n");
    return exit_error_code;
  }

  // Don't need to deallocate 'pointer_of_pointers' because the memory will be loaded and free in the wasm
  //

  uint32_t offset = 9;
  unsigned char rvec[offset];
  uint32_t retsInt = WasmEdge_ValueGetI32(rets[0]);
  WasmEdge_MemoryInstanceGetData(MemoryCxt, rvec, retsInt, offset);
  deallocate(VMCxt, retsInt, offset);

  unsigned char flag = rvec[0];
  if (flag == 0) {
    unsigned char ret_pointer[5];
    splice(rvec, 1, 5, ret_pointer);

    unsigned char ret_len[5];
    splice(rvec, 5, 9, ret_len);

    parse_result(VMCxt, MemoryCxt, ret_pointer, ret_len);
  } else {
    printf("Error: parsing result failed\n");
    return exit_error_code;
  }

  return exit_success_code;
}

//
//
//
// int myInt = 123; // Your integer value
// unsigned char *ptr = reinterpret_cast<unsigned char*>(&myInt);
// for (int i = 0; i < sizeof(int); ++i) {
//     std::cout << "Byte " << i << ": " << static_cast<int>(ptr[i]) << std::endl;
// }
//
// The output you're seeing is expected for a little-endian system, where the least significant byte comes first in memory. In your case, it appears that the integer 123 is represented in memory as follows:
//
// Byte 0: 123
// Byte 1: 0
// Byte 2: 0
// Byte 3: 0
// This output indicates that the integer 123 is stored in memory using 4 bytes, where the least significant byte (LSB) is 123 and the other bytes are 0. This is consistent with little-endian byte ordering, where the least significant byte comes first in memory.

// So yes, it's okay, and it reflects the memory layout of the integer 123 on your system.