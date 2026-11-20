// Extracto de RDA BLOCKS 0.3.32 · © Robodacta. Solo para mostrar el diseño; el proyecto completo no es público.
// Archivo original: src/vm/minisumo-compiler.js (fragmento)

// Traduce los bloques de los kits con Arduino Nano (Mini Sumo, KIT 1201) a bytecode de la Robodacta VM.
// El comportamiento debe ser el mismo que el del código Arduino que genera src/arduino.js:
// mismos valores, mismo orden y «y»/«o» con cortocircuito como en C++.
// …

 for (let b = first; b; b = b.getNextBlock()) if (b.isEnabled()) statement(ctx, b);
}

function motors(ctx, left, right) {
 ctx.asm.number(left).number(right).native(MINISUMO.MOTORS);
}

function statement(ctx, b) {
 const {asm} = ctx;
 switch (b.type) {
  case 'rd_move': return motors(ctx, ...MOVES[b.getFieldValue('DIR')](num(b, 'SPEED')));
  case 'rd_stop': return motors(ctx, 0, 0);
  case 'rd_motors': return motors(ctx, num(b, 'LEFT'), num(b, 'RIGHT'));
  case 'rd_wait': asm.number(Math.round(num(b, 'TIME') * 1000)).emit(OP.WAIT); return;
  case 'rd_led': asm.number(b.getFieldValue('STATE') === 'HIGH' ? 1 : 0).native(MINISUMO.LED); return;
  case 'rd_tone': asm.number(num(b, 'FREQ')).number(num(b, 'MS')).native(MINISUMO.TONE); return;
  case 'rd_print': value(ctx, b, 'VALUE'); asm.native(NATIVE.PRINT); return;
  case 'rd_if': {
   const end = asm.label();
   value(ctx, b, 'COND');
   asm.jump(OP.JF, end);
   statements(ctx, b.getInputTargetBlock('DO'));
   asm.mark(end);
   return;
  }
  case 'rd_if_else': {
   const other = asm.label(), end = asm.label();
   value(ctx, b, 'COND');
   asm.jump(OP.JF, other);
   statements(ctx, b.getInputTargetBlock('DO'));
   asm.jump(OP.JMP, end).mark(other);
   statements(ctx, b.getInputTargetBlock('ELSE'));
   asm.mark(end);
   return;
  }
  // … (repetir, variables, sensores y expresiones)
 }
}
