#include "redismodule.h"
#include <random>
#include <cmath>

extern "C" {
//  int RandomUnif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

//  int RandomExp_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

  int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
}
 
// Random device and Mersenne Twister engine are shared across commands 
std::random_device rd;
std::mt19937 gen(rd());

/* RANDOM.DUNIF START END */
int RandomDUnif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc != 3) return RedisModule_WrongArity(ctx);
  long long start, end;
  if (
      (RedisModule_StringToLongLong(argv[1],&start) != REDISMODULE_OK) ||
      (RedisModule_StringToLongLong(argv[2],&end) != REDISMODULE_OK) ||
      (start > end) 
      ) 
    return RedisModule_ReplyWithError(ctx,"ERR invalid range");

  std::uniform_int_distribution<long long> rdunif(start,end);
  RedisModule_ReplyWithLongLong(ctx,rdunif(gen));
  return REDISMODULE_OK;
}

/* RANDOM.UNIF START END */
int RandomUnif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc != 3) return RedisModule_WrongArity(ctx);
  double start, end;
  if (
      (RedisModule_StringToDouble(argv[1],&start) != REDISMODULE_OK) ||
      (RedisModule_StringToDouble(argv[2],&end) != REDISMODULE_OK) ||
      (start > end) 
      ) 
    return RedisModule_ReplyWithError(ctx,"ERR invalid range");

  std::uniform_real_distribution<double> runif(start,end);
  RedisModule_ReplyWithDouble(ctx,runif(gen));
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
/* RANDOM.LUNIF KEY COUNT START END */
int RandomLUnif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  double start, end;
  if (argc != 5) return RedisModule_WrongArity(ctx);
  if (RedisModule_StringToDouble(argv[3],&start) != REDISMODULE_OK)
    return RedisModule_ReplyWithError(ctx,"ERR invalid start");
  if (RedisModule_StringToDouble(argv[4],&end) != REDISMODULE_OK)
    return RedisModule_ReplyWithError(ctx,"ERR invalid end");

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
  std::uniform_real_distribution<double> runif(start,end);
  while (count-- > 0)
  {
    RedisModuleString *ele;
    /* Make string with the same 19 digita precision of ReplyWithDouble */
    ele=RedisModule_CreateStringPrintf(ctx,"%.19f",runif(gen));
    RedisModule_ListPush(key,REDISMODULE_LIST_HEAD,ele);
    RedisModule_FreeString(ctx,ele);
  }
 
  size_t len = RedisModule_ValueLength(key);
  RedisModule_CloseKey(key);
  RedisModule_ReplyWithLongLong(ctx, len);
  return REDISMODULE_OK;
}

/* RANDOM.LNORM KEY COUNT [MEAN=0.0] [STDDEV=1.0] */
int RandomLNorm_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  double mean=0.0, sd=1.0;
  if (argc > 5) return RedisModule_WrongArity(ctx);
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

int RandomHist_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc > 4) return RedisModule_WrongArity(ctx);

  long long slots=10;
  long long col=0;

  if (argc >= 3) 
  {
    if (RedisModule_StringToLongLong(argv[2],&slots) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid hist size");
  }
  if (argc == 4) 
  {
    if (RedisModule_StringToLongLong(argv[3],&col) != REDISMODULE_OK)
      return RedisModule_ReplyWithError(ctx,"ERR invalid columns size");
  }

  /* Check key type */
  RedisModuleKey * key = ( RedisModuleKey *) 
    RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_LIST)
  {
    RedisModule_CloseKey(key);
    return RedisModule_ReplyWithError(ctx,"ERR key not of List type");
  }
  RedisModule_CloseKey(key);

  long long hist[slots];
  for (auto i=0; i < slots; i++) hist[i]=0;
  RedisModuleCallReply *reply;
  reply = RedisModule_Call(ctx,"LRANGE","scc",argv[1],"0","-1");

  if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_NULL ||
      RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ERROR )
  {
    RedisModule_FreeCallReply(reply);
    return RedisModule_ReplyWithError(ctx,"ERR error in key");
  }

  size_t len;
  double min=0, max=0;
  if (RedisModule_CallReplyType(reply) == REDISMODULE_REPLY_ARRAY)
  {
    len = RedisModule_CallReplyLength(reply);
    if (len == 0)
    {
      RedisModule_FreeCallReply(reply);
      return REDISMODULE_OK;
    }
    for (auto i=0; i < len; i++)
    {
      RedisModuleCallReply *subreply;
      subreply = RedisModule_CallReplyArrayElement(reply,i);
      double e;
      RedisModuleString *ele = RedisModule_CreateStringFromCallReply(subreply);
      if (RedisModule_StringToDouble(ele,&e)==REDISMODULE_ERR)
      {
        RedisModule_FreeString(ctx,ele);
        RedisModule_FreeCallReply(reply);
        return RedisModule_ReplyWithError(ctx,"ERR bad list value");
      }
      RedisModule_FreeString(ctx,ele);
      if (i==0) min=max=e;
      if (e < min) min=e;
      if (e > max) max=e;
    }
    double range=max-min;
    for (auto i=0; i < len; i++)
    {
      RedisModuleCallReply *subreply;
      subreply = RedisModule_CallReplyArrayElement(reply,i);
      double e;
      RedisModuleString *ele = RedisModule_CreateStringFromCallReply(subreply);
      RedisModule_StringToDouble(ele,&e);
      RedisModule_FreeString(ctx,ele);
      long long slot=(long long) std::floor(((e-min)/range)*slots);
      if (slot==slots) slot--; /* max value goes to last slot */
      hist[slot]++;
    }

  }
  RedisModule_FreeCallReply(reply);

  RedisModule_ReplyWithArray(ctx,slots);
  if (col==0)
  {
    for (auto i=0; i < slots; i++)
      RedisModule_ReplyWithLongLong(ctx,hist[i]);
  }
  else
  {
    char s[col];
    for (auto i=0; i < col; i++)
      s[i] = {'*'};
    double hmax=hist[0];
    for (auto i=0; i < slots; i++)
      if (hmax < hist[i]) 
        hmax = hist[i];
    for (auto i=0; i < slots; i++)
      RedisModule_ReplyWithStringBuffer(ctx,s,std::floor(((double) hist[i])/hmax*col));
  }
  return REDISMODULE_OK;
}


int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx,"random",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.dunif",
        RandomDUnif_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.unif",
        RandomUnif_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.lunif",
        RandomLUnif_RedisCommand,"random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.norm",
        RandomNorm_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.lnorm",
        RandomLNorm_RedisCommand,"random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.exp",
        RandomExp_RedisCommand,"readonly random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.lexp",
        RandomLExp_RedisCommand,"random",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"random.hist",
        RandomHist_RedisCommand,"readonly",0,0,0) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
