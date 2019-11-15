#ifndef _ICAMERAMIRRORALGO_H_
#define _ICAMERAMIRRORALGO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <include/link_api/system.h>
typedef struct
{
 UInt32 xxx;
} Alg_CameraMirror_Obj;
typedef struct
{
 UInt32 xxxx;
} Alg_CameraMirrorCreateParams;
typedef struct
{
 UInt32 xxx;
 /**< Any parameters which can be used to alter the algorithm behavior */
} Alg_CameraMirrorControlParams;
Alg_CameraMirror_Obj * Alg_CameraMirrorCreate(
 Alg_CameraMirrorCreateParams *pCreateParams);
Int32 Alg_CameraMirrorProcess(Alg_CameraMirror_Obj *algHandle,
 UInt32 *inPtr[],
UInt32 width,
UInt32 height,
 UInt32 inPitch[],
 UInt32 dataFormat
 );
Int32 Alg_CameraMirrorControl(Alg_CameraMirror_Obj *pAlgHandle,Alg_CameraMirrorControlParams *pControlParams);
Int32 Alg_CameraMirrorDelete(Alg_CameraMirror_Obj *pAlgHandle);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
