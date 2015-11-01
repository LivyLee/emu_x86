
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "modrm.h"
#include "emulator_function.h"

void parse_modrm(Emulator* emu, ModRM* modrm)
{
  uint8_t code;

  assert(emu != NULL && modrm != NULL);

  memset(modrm, 0, sizeof(ModRM)); // 全部を 0 に初期化

  code = get_code8(emu, 0);
  modrm->mod = ((code & 0xC0) >> 6);
  modrm->opecode = ((code & 0x38) >> 3);
  modrm->rm = code & 0x07;

  emu->eip += 1;

  if (modrm->mod != 3 && modrm->rm == 4) {
    modrm->sib = get_code8(emu, 0);
    emu->eip += 1;
  }

  if ((modrm->mod == 0 && modrm->rm == 5) || modrm->mod == 2) {
    modrm->disp32 = get_sign_code32(emu, 0);
    emu->eip += 4;
  } else if (modrm->mod == 1) {
    modrm->disp8 = get_sign_code8(emu, 0);
    emu->eip += 1;
  }
}

uint32_t eval_sib(Emulator* emu, ModRM* modrm)
{
  uint8_t sib = modrm->sib;
  uint8_t scale = ((sib & 0xC0) >> 6);
  uint8_t base  = ((sib & 0x38) >> 3);
  uint8_t index = (sib & 0x03);
  uint32_t r32b, r32i;

  if (base == 5) {
    r32b = 0;
  } else {
    r32b = get_register32(emu, base);
  }

  if (index == 4) {
    return r32b;
  } else {
    r32i = get_register32(emu, index);
  }

  if (scale == 0) {
    return r32b;
  } else if (scale == 1) {
    return r32b + r32i * 2;
  } else if (scale == 2) {
    return r32b + r32i * 4;
  } else {  /* scale == 3 */
    return r32b + r32i * 8;
  }
}

uint32_t calc_memory_address(Emulator* emu, ModRM* modrm)
{
  /* レジスタ間接参照型 */
  if (modrm->mod == 0) {
    if (modrm->rm == 4) {
      // printf("not implemented ModRM mod = 0, rm = 4\n");
      // exit(0);
      return eval_sib(emu, modrm);
    } else if (modrm->mod == 5) {
      return modrm->disp32;
    } else {
      return get_register32(emu, modrm->rm);
    }
  /* レジスタ間接参照 + 8bitディスプレースメント型 */
  } else if (modrm->mod == 1) {
    if (modrm->rm == 4) {
      // printf("not implemented ModRM mod = 1, rm = 4\n");
      // exit(0);
      return eval_sib(emu, modrm) + modrm->disp8;
    } else {
      return get_register32(emu, modrm->rm) + modrm->disp8;
    }
  /* レジスタ間接参照 + 32bitディスプレースメント型 */
  } else if (modrm->mod == 2) {
    if (modrm->rm == 4) {
      // printf("not implemented ModRM mod = 2, rm = 4\n");
      // exit(0);
      return eval_sib(emu, modrm) + modrm->disp32;
    } else {
      return get_register32(emu, modrm->rm) + modrm->disp32;
    }
  /* レジスタ直接参照型 */
  } else {
    printf("not implemented ModRM mod\n");
    exit(0);
  }
}

uint32_t get_rm32(Emulator* emu, ModRM* modrm)
{
  if (modrm->mod == 3) {
    return get_register32(emu, modrm->rm);
  } else {
    uint32_t address = calc_memory_address(emu, modrm);
    return get_memory32(emu, address);
  }
}

void set_rm32(Emulator* emu, ModRM* modrm, uint32_t value)
{
  if (modrm->mod == 3) {
    set_register32(emu, modrm->rm, value);
  } else {
    uint32_t address = calc_memory_address(emu, modrm);
    set_memory32(emu, address, value);
  }
}

uint32_t get_r32(Emulator* emu, ModRM* modrm)
{
  return get_register32(emu, modrm->reg_index);
}

void set_r32(Emulator* emu, ModRM* modrm, uint32_t value)
{
  set_register32(emu, modrm->reg_index, value);
}
