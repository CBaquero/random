#include "redismodule.h"
#include <random>

extern "C" {
  int RandomUnif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

  int RandomExp_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

  int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
}
 
// Random device and Mersenne Twister engine are shared across commands 
std::random_device rd;
std::mt19937 gen(rd());

int RandomUnif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc != 3) return RedisModule_WrongArity(ctx);
  long long start, end;
  if (
      (RedisModule_StringToLongLong(argv[1],&start) != REDISMODULE_OK) ||
      (RedisModule_StringToLongLong(argv[2],&end) != REDISMODULE_OK) ||
      (start > end) 
      ) 
    return RedisModule_ReplyWithError(ctx,"ERR invalid range");

  std::uniform_int_distribution<long long> runif(start,end);
  RedisModule_ReplyWithLongLong(ctx,runif(gen));
  return REDISMODULE_OK;
}

int RandomExp_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  double lambda;
  if (argc > 2) return RedisModule_WrongArity(ctx);
  if (argc == 2)
  {
    if (RedisModule_StringToDouble(argv[1],&lambda) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid lambda");
  }
  else // If no parameter, use default lambda of 1
    lambda=1.0;
  std::exponential_distribution<double> rexp(lambda);
  RedisModule_ReplyWithDouble(ctx, rexp(gen));
  return REDISMODULE_OK;
}


int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx,"random",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.unif",
        RandomUnif_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.exp",
        RandomExp_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;


    return REDISMODULE_OK;
}
