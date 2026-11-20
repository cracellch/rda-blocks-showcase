// Extracto de RDA BLOCKS 0.3.32 · © Robodacta. Solo para mostrar el diseño; el proyecto completo no es público.
// Archivo original: firmware/robodacta_nano_vm/rd_vm.cpp (fragmento)
// El mismo intérprete existe en JavaScript (src/sim/vm.js) para el Simulador 3D;
// tests/sim.test.js comprueba que los dos producen exactamente los mismos eventos.

#include "rd_vm.h"

#include <math.h>
#include <string.h>

namespace rdvm {

// …

Machine::Step Machine::step(Host &host) {
  opAddress_ = pc_;
  uint8_t op;
  if (!fetch8(host, op)) return STEP_HALT;
  switch (op) {
    case OP_END:
      // «al encender» terminó o una vuelta de «por siempre» terminó: empieza la siguiente.
      if (cp_ != 0) return halt(ERR_CALLS);
      if (state_ == STATE_SETUP) state_ = STATE_LOOP;
      sp_ = 0;
      pc_ = loop_;
      return STEP_YIELD;
    case OP_PUSH_I8: {
      uint8_t v;
      if (!fetch8(host, v)) return STEP_HALT;
      return push((float) (int8_t) v) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_PUSH_I16: {
      uint16_t v;
      if (!fetch16(host, v)) return STEP_HALT;
      return push((float) (int16_t) v) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_PUSH_F32: {
      uint8_t bytes[4];
      for (uint8_t i = 0; i < 4; i++)
        if (!fetch8(host, bytes[i])) return STEP_HALT;
      float v;
      memcpy(&v, bytes, 4);  // IEEE 754 little-endian en AVR y en la PC
      return push(v) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_POP: {
      float v;
      return pop(v) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_DUP: {
      float v;
      if (!pop(v) || !push(v) || !push(v)) return STEP_HALT;
      return STEP_CONTINUE;
    }
    case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
    case OP_LT: case OP_GT: case OP_LE: case OP_GE: case OP_EQ: case OP_NE:
      return binary(op) ? STEP_CONTINUE : STEP_HALT;
    case OP_NEG: case OP_NOT: case OP_BOOL: {
      float v;
      if (!pop(v)) return STEP_HALT;
      float r = op == OP_NEG ? -v : op == OP_NOT ? (v == 0 ? 1.0f : 0.0f) : (v != 0 ? 1.0f : 0.0f);
      return push(r) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_JMP: {
      uint16_t a;
      if (!fetch16(host, a) || !jumpTo(a)) return STEP_HALT;
      return STEP_CONTINUE;
    }
    case OP_JF: case OP_JT: {
      uint16_t a;
      float v;
      if (!fetch16(host, a) || !pop(v)) return STEP_HALT;
      bool truth = v != 0;
      if (truth == (op == OP_JT) && !jumpTo(a)) return STEP_HALT;
      return STEP_CONTINUE;
    }
    case OP_LOAD: case OP_STORE: {
      uint8_t index;
      if (!fetch8(host, index)) return STEP_HALT;
      if (index >= VARIABLES) return halt(ERR_VARIABLE);
      if (op == OP_LOAD) return push(vars_[index]) ? STEP_CONTINUE : STEP_HALT;
      return pop(vars_[index]) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_CALL: {
      uint16_t a;
      if (!fetch16(host, a)) return STEP_HALT;
      if (cp_ >= CALL_DEPTH) return halt(ERR_CALLS);
      calls_[cp_++] = pc_;
      return jumpTo(a) ? STEP_CONTINUE : STEP_HALT;
    }
    case OP_RET:
      if (cp_ == 0) return halt(ERR_CALLS);
      pc_ = calls_[--cp_];
      return STEP_CONTINUE;
    case OP_WAIT: {
      float ms;
      if (!pop(ms)) return STEP_HALT;
      wait(host.now(), ms > 0 ? (uint32_t) ms : 0);
      return STEP_YIELD;
    }
    case OP_NATIVE: {
      uint8_t id;
      if (!fetch8(host, id)) return STEP_HALT;
      if (!host.native(id, *this)) return halt(ERR_NATIVE);
      return state_ == STATE_ERROR ? STEP_HALT : STEP_CONTINUE;
    }
    default:
      return halt(ERR_OPCODE);
  }
}

}  // namespace rdvm
