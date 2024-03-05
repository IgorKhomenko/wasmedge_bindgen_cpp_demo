#include <wasmedge/wasmedge.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <any>

const int exit_error_code = 1;
const int exit_success_code = 0;

const char *wasm_file_path = "../module/target/wasm32-wasi/release/funcs.wasm";

enum RetTypes {
	U8 = 1,
	I8 = 2,
	U16 = 3,
	I16 = 4,
	U32 = 5,
	I32 = 6,
	U64 = 7,
	I64 = 8,
	F32 = 9,
	F64 = 10,
	Bool = 11,
	Char = 12,
	U8Array = 21,
	I8Array = 22,
	U16Array = 23,
	I16Array = 24,
	U32Array = 25,
	I32Array = 26,
	U64Array = 27,
	I64Array = 28,
	String = 31,
};

void splice(const unsigned char* array, int start, int end, unsigned char* spliced_array) {
  for (int i = start; i < end; ++i) {
    *spliced_array++ = array[i];
  }
}

void printBytes(unsigned char *vec, int len, char *name) {
  for (int i = 0; i < len; ++i) {
    std::cout << "[" << name << "] Byte: " << i << ": " << static_cast<int>(vec[i]) << std::endl;
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

std::vector<int> settle(WasmEdge_VMContext *VMCxt, WasmEdge_MemoryInstanceContext *MemoryCxt, int input) {
  int length_of_input = 1;
	int pointer = allocate(VMCxt, length_of_input * 4);

  unsigned char bytes[sizeof(int)];  
  std::memcpy(bytes, &input, sizeof(int));

  WasmEdge_MemoryInstanceSetData(MemoryCxt, bytes, pointer, 4);

  std::vector<int> res; 
  res.push_back(pointer);
  res.push_back(length_of_input);

  return res;
}

std::vector<int> settle(WasmEdge_VMContext *VMCxt, WasmEdge_MemoryInstanceContext *MemoryCxt, float input) {
  printf("settle float %f\n", input);

  int length_of_input = 1;
	int pointer = allocate(VMCxt, length_of_input * 4);

  unsigned char bytes[sizeof(float)];  
  std::memcpy(bytes, &input, sizeof(float));

  WasmEdge_MemoryInstanceSetData(MemoryCxt, bytes, pointer, 4);

  std::vector<int> res; 
  res.push_back(pointer);
  res.push_back(length_of_input);

  return res;
}

std::vector<std::any> parse_result(WasmEdge_VMContext *VMCxt, WasmEdge_MemoryInstanceContext *MemoryCxt, unsigned char *ret_pointer, unsigned char *ret_len) {
  int size = static_cast<int>(*ret_len);

  int retPointer;
  std::memcpy(&retPointer, ret_pointer, sizeof(int));

  // printf("retPointer: %d\n", retPointer);
  // printf("size: %d\n", size);

  int p_data_len = size * 3 * 4;
  unsigned char p_data[p_data_len];
  WasmEdge_MemoryInstanceGetData(MemoryCxt, p_data, retPointer, p_data_len);
  deallocate(VMCxt, retPointer, p_data_len);

  // printBytes(p_data, p_data_len, "p_data");

  std::vector<int> p_values; 
  // p_values.reserve(size * 3);
  for (int i = 0; i < (size * 3); ++i) {
    unsigned char p_data_slice[4];
    splice(p_data, i*4, (i+1)*4, p_data_slice);

    // printf("i: %d. %d - %d\n", i, i*4, (i+1)*4);  
    // printBytes(p_data_slice, 4, "p_data_slice");

    int p_data_slice_int;
    std::memcpy(&p_data_slice_int, p_data_slice, sizeof(int));
    // printf("p_data_slice_int: %d\n", p_data_slice_int);  
    p_values.push_back(p_data_slice_int);
  }

  std::vector<std::any> results; 
  // results.reserve(size);
  for (int i = 0; i < size; ++i) {
      const int len = p_values[i*3+2];
      unsigned char bytes[len];
      WasmEdge_MemoryInstanceGetData(MemoryCxt, bytes, p_values[i*3], len);
      deallocate(VMCxt, p_values[i*3], len);

      const int retType = p_values[i*3+1];
      // printf("retType: %d\n", retType);

      switch (retType) {
        case RetTypes::String: {
          std::string bytesString((char *)bytes);
          printf("val string: %s\n", bytesString.c_str());
          results.push_back(bytesString);
          break;
        }
        case RetTypes::I32: {
          int val;
          std::memcpy(&val, bytes, sizeof(int));
          printf("val int: %d\n", val);
          results.push_back(val);
          break;
        }
        case RetTypes::F32: {
          float val;
          std::memcpy(&val, bytes, sizeof(float));
          printf("val float: %f\n", val);
          results.push_back(val);
          break;
        }
        case RetTypes::Bool: {
          bool val;
          std::memcpy(&val, bytes, sizeof(bool));
          printf("val bool: %d\n", val);
          results.push_back(val);
          break;
        }
      }
  }

  return results;
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



  std::vector<std::any> inputs; 
  inputs.push_back("bob");
  inputs.push_back(11);
  inputs.push_back(36.6);
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
    std::vector<int> sr;
    if (inp.type() == typeid(int)) {
      sr = settle(VMCxt, MemoryCxt, std::any_cast<int>(inp));
    } else if (inp.type() == typeid(double)) {
      double val = std::any_cast<double>(inp);
      sr = settle(VMCxt, MemoryCxt, (float)val);
    } else if (inp.type() == typeid(const char*)) {
      const char* val = std::any_cast<const char*>(inp);
      sr = settle(VMCxt, MemoryCxt, std::string(val));
    } else {
       // TODO
    }

    int pointer = sr[0];
    //
    unsigned char *ucPointerLittleEndian = reinterpret_cast<unsigned char*>(&pointer);
    // printBytes(ucPointerLittleEndian, "ucPointerLittleEndian");
    //
    WasmEdge_MemoryInstanceSetData(MemoryCxt,  ucPointerLittleEndian, pointer_of_pointers + pos * 4 * 2, sizeof(ucPointerLittleEndian));

    int length_of_input = sr[1];
    //
    unsigned char *ucLenghtOfInputLittleEndian = reinterpret_cast<unsigned char*>(&length_of_input);
    // printBytes(ucLenghtOfInputLittleEndian, "ucLenghtOfInputLittleEndian");
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

  uint32_t size = 9;
  unsigned char rvec[size];
  uint32_t retsInt = WasmEdge_ValueGetI32(rets[0]);
  WasmEdge_MemoryInstanceGetData(MemoryCxt, rvec, retsInt, size);
  deallocate(VMCxt, retsInt, size);

  // printBytes(rvec, size, "rvec");

  unsigned char flag = rvec[0];
  if (flag == 0) {
    unsigned char ret_pointer[4];
    splice(rvec, 1, 5, ret_pointer);
    // printBytes(ret_pointer, 4, "ret_pointer");

    unsigned char ret_len[4];
    splice(rvec, 5, 9, ret_len);
    // printBytes(ret_len, 4, "ret_len");

    std::vector<std::any> results = parse_result(VMCxt, MemoryCxt, ret_pointer, ret_len);
  } else {
    printf("Error: parsing result failed\n");
    return exit_error_code;
  }

  return exit_success_code;
}