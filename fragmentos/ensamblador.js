// Extracto de RDA BLOCKS 0.3.32 · © Robodacta. Solo para mostrar el diseño; el proyecto completo no es público.
// Archivo original: src/vm/rdvm.js (fragmento)

export const OP = Object.freeze({
 END:0x00, PUSH_I8:0x01, PUSH_I16:0x02, PUSH_F32:0x03, POP:0x04, DUP:0x05,
 ADD:0x10, SUB:0x11, MUL:0x12, DIV:0x13, MOD:0x14, NEG:0x15,
 LT:0x18, GT:0x19, LE:0x1A, GE:0x1B, EQ:0x1C, NE:0x1D, NOT:0x1E, BOOL:0x1F,
 JMP:0x20, JF:0x21, JT:0x22, LOAD:0x28, STORE:0x29, CALL:0x30, RET:0x31, WAIT:0x38, NATIVE:0x40,
});
// …

// Ensamblador con etiquetas: los saltos se resuelven al terminar.
export class Assembler {
 constructor() { this.bytes = []; this.fixups = []; this.labels = new Map(); this.count = 0; }
 get address() { return this.bytes.length; }
 emit(...bytes) { for (const b of bytes) this.bytes.push(b & 0xFF); return this; }
 label() { return this.count++; }
 mark(label) { this.labels.set(label, this.address); return this; }
 jump(op, label) { this.emit(op); this.fixups.push({at:this.address, label}); return this.emit(0, 0); }
 native(id) { return this.emit(OP.NATIVE, id); }
 number(value) {
  if (!Number.isFinite(value)) throw new Error('Hay un número no válido en el programa.');
  if (Number.isInteger(value) && value >= -128 && value <= 127) return this.emit(OP.PUSH_I8, value);
  if (Number.isInteger(value) && value >= -32768 && value <= 32767) return this.emit(OP.PUSH_I16, ...le16(value & 0xFFFF));
  const view = new DataView(new ArrayBuffer(4));
  view.setFloat32(0, value, true);
  return this.emit(OP.PUSH_F32, ...new Uint8Array(view.buffer));
 }
 finish() {
  for (const {at, label} of this.fixups) {
   const address = this.labels.get(label);
   if (address === undefined) throw new Error('Etiqueta sin resolver en el programa.');
   [this.bytes[at], this.bytes[at + 1]] = le16(address);
  }
  return Uint8Array.from(this.bytes);
