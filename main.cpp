#include <wasmedge/wasmedge.h>
#include <stdio.h>
#include <iostream>
#include <vector>

const int exit_error_code = 1;
const int exit_success_code = 0;

const char *wasm_file_path = "../module/target/wasm32-wasi/release/funcs.wasm";

int main() {

  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt, WasmEdge_HostRegistration_Wasi);

  WasmEdge_StoreContext *StoreCxt = WasmEdge_StoreCreate();
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, StoreCxt);

  // add args/envs/preopens
  // this line start display logs from host functions 
  WasmEdge_ModuleInstanceContext *WasiCxt = WasmEdge_VMGetImportModuleContext(VMCxt, WasmEdge_HostRegistration_Wasi);
  WasmEdge_ModuleInstanceInitWASI(WasiCxt,  NULL, 0, NULL, 0,  NULL, 0);

  WasmEdge_Value P[2], R[1]; 
  WasmEdge_String FuncName;
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
  //
  int inputs_count = inputs.size();
  //
  int pointer_of_pointers;

  // alloc a space for input args
  P[0] = WasmEdge_ValueGenI32(inputs_count * 4 * 2);
  FuncName = WasmEdge_StringCreateByCString("allocate");
  // Don't need to deallocate because the memory will be loaded and free in the wasm
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 1, R, 1);
  WasmEdge_StringDelete(FuncName);

  if (WasmEdge_ResultOK(Res)) {
    pointer_of_pointers = WasmEdge_ValueGetI32(R[0]);
    printf("[allocate] Ok: %d\n", pointer_of_pointers);
  } else {
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


  // // WasmEdge_MemoryInstanceSetData(WasmEdge_MemoryInstanceContext *Cxt,
  // //                              const uint8_t *Data, const uint32_t Offset,
  // //                              const uint32_t Length);


  // Run func
  P[0] = WasmEdge_ValueGenI32(pointer_of_pointers); // params_pointer: *mut u32
  P[1] = WasmEdge_ValueGenI32(inputs_count); // params_count: i32
  FuncName = WasmEdge_StringCreateByCString("say");
  Res = WasmEdge_VMExecute(VMCxt, FuncName, P, 2, R, 1);
  WasmEdge_StringDelete(FuncName);
  if (WasmEdge_ResultOK(Res)) {
    printf("[say] Ok: %d\n", WasmEdge_ValueGetI32(R[0]));
  } else {
    printf("[say] Error\n");
    return exit_error_code;
  }

  return exit_success_code;
}