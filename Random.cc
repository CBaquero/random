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

/* RANDOM.UNIF START END */
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

/* RANDOM.NORM [MEAN=0.0] [STDDEV=1.0] */
int RandomNorm_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  double mean=0.0, sd=1.0;
  if (argc > 3) return RedisModule_WrongArity(ctx);
  if (argc >= 2) /* Get mean */
  {
    if (RedisModule_StringToDouble(argv[1],&mean) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid mean");
  }
  if (argc == 3) /* get stddev */
  {
    if (RedisModule_StringToDouble(argv[2],&sd) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid standard deviation");
  }

  std::normal_distribution<double> rnorm(mean,sd);
  RedisModule_ReplyWithDouble(ctx, rnorm(gen));
  return REDISMODULE_OK;
}

/* RANDOM.LNORM KEY COUNT [MEAN=0.0] [STDDEV=1.0] */
int RandomLNorm_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  double mean=0.0, sd=1.0;
  if (argc > 5 || argc < 3) return RedisModule_WrongArity(ctx);
  if (argc >= 4) /* Get mean */
  {
    if (RedisModule_StringToDouble(argv[3],&mean) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid mean");
  }
  if (argc == 5) /* get stddev */
  {
    if (RedisModule_StringToDouble(argv[4],&sd) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid standard deviation");
  }

  /* Open key */
  RedisModuleKey *key = (RedisModuleKey *) RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
 /* Key must be empty or list */
  if ((RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_LIST &&
       RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY)) 
  {
     RedisModule_CloseKey(key);
     return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  /* Get count */
  long long count;
  if ((RedisModule_StringToLongLong(argv[2],&count) != REDISMODULE_OK) ||
        (count < 0)) 
  {
     RedisModule_CloseKey(key);
     return RedisModule_ReplyWithError(ctx,"ERR invalid count");
  }

  /* Push count randoms */
  std::normal_distribution<double> rnorm(mean,sd);
  while (count-- > 0)
  {
    RedisModuleString *ele;
    /* Make string with the same 19 digita precision of ReplyWithDouble */
    ele=RedisModule_CreateStringPrintf(ctx,"%.19f",rnorm(gen));
    RedisModule_ListPush(key,REDISMODULE_LIST_HEAD,ele);
    RedisModule_FreeString(ctx,ele);
  }
 
  size_t len = RedisModule_ValueLength(key);
  RedisModule_CloseKey(key);
  RedisModule_ReplyWithLongLong(ctx, len);
  return REDISMODULE_OK;
}

/* RANDOM.EXP [LAMBDA=1.0] */
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

/* RANDOM.LEXP KEY COUNT [LAMBDA=1.0] */
int RandomLExp_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  double lambda;
  if (argc < 3 || argc > 4) return RedisModule_WrongArity(ctx);
  if (argc == 4)
  {
    if (RedisModule_StringToDouble(argv[3],&lambda) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid lambda");
  }
  else // If no parameter, use default lambda of 1
    lambda=1.0;

  /* Open key */
  RedisModuleKey *key = (RedisModuleKey *) RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
 /* Key must be empty or list */
  if ((RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_LIST &&
       RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY)) 
  {
     RedisModule_CloseKey(key);
     return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  /* Get count */
  long long count;
  if ((RedisModule_StringToLongLong(argv[2],&count) != REDISMODULE_OK) ||
        (count < 0)) 
  {
     RedisModule_CloseKey(key);
     return RedisModule_ReplyWithError(ctx,"ERR invalid count");
  }

  /* Push count randoms */
  std::exponential_distribution<double> rexp(lambda);
  while (count-- > 0)
  {
    RedisModuleString *ele;
    /* Make string with the same 19 digita precision of ReplyWithDouble */
    ele=RedisModule_CreateStringPrintf(ctx,"%.19f",rexp(gen));
    RedisModule_ListPush(key,REDISMODULE_LIST_HEAD,ele);
    RedisModule_FreeString(ctx,ele);
  }
 
  size_t len = RedisModule_ValueLength(key);
  RedisModule_CloseKey(key);
  RedisModule_ReplyWithLongLong(ctx, len);
  return REDISMODULE_OK;
}



int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx,"random",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.unif",
        RandomUnif_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.norm",
        RandomNorm_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.lnorm",
        RandomLNorm_RedisCommand,"random",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.exp",
        RandomExp_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.lexp",
        RandomLExp_RedisCommand,"random",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;



    return REDISMODULE_OK;
}
