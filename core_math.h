#pragma once
#include "interpreter.h"

#define commonTricketry(FUNCTION) \
	float64 FUNCTION ## _dd(float64 left, float64 right); \
	float64 FUNCTION ## _df(float64 left, float32 right); \
	float64 FUNCTION ## _di(float64 left, int64 right); \
	float64 FUNCTION ## _du(float64 left, uint64 right); \
	float64 FUNCTION ## _fd(float32 left, float64 right); \
	float32 FUNCTION ## _ff(float32 left, float32 right);\
	float32 FUNCTION ## _fi(float32 left, int64 right);\
	float32 FUNCTION ## _fu(float32 left, uint64 right);\
	float64 FUNCTION ## _id(int64 left, float64 right);\
	float32 FUNCTION ## _if(int64 left, float32 right);\
	int64   FUNCTION ## _ii(int64 left, int64 right);\
	int64   FUNCTION ## _iu(int64 left, uint64 right);\
	float64 FUNCTION ## _ud(uint64 left, float64 right);\
	float32 FUNCTION ## _uf(uint64 left, float32 right);\
	int64   FUNCTION ## _ui(uint64 left, int64 right);\
	uint64  FUNCTION ## _uu(uint64 left, uint64 right);

namespace Core {

	uint64 bitOR(uint64 left, uint64 right);
	uint64 bitAND(uint64 left, uint64 right);
	uint64 bitXOR(uint64 left, uint64 right);

	commonTricketry(add);
	commonTricketry(sub);
	commonTricketry(mul);
	commonTricketry(div);

	uint64 mod_uu(uint64 left, uint64 right);
	int64 mod_ui(uint64 left, int64 right);
	int64 mod_iu(int64 left, uint64 right);
	int64 mod_ii(int64 left, int64 right);
}

#undef commonTricketry(FUNCTION)