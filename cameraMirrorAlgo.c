#include "iCameraMirrorAlgo.h"
Alg_CameraMirror_Obj * Alg_CameraMirrorCreate(
 Alg_CameraMirrorCreateParams *pCreateParams)
{
 Alg_CameraMirror_Obj * pAlgHandle;
 pAlgHandle = (Alg_CameraMirror_Obj *) malloc(sizeof(Alg_CameraMirror_Obj));
 UTILS_assert(pAlgHandle != NULL);
 return pAlgHandle;
}

Int32 Alg_CameraMirrorProcess(Alg_CameraMirror_Obj *algHandle,
 UInt32 *inPtr[],
UInt32 width,
UInt32 height,
UInt32 inPitch[],
UInt32 dataFormat
 )
{
 Int32 rowIdx;
 Int32 colIdx;
 UInt32 wordWidth;

 UInt32 *inputPtr;
 /* Uncomment the variables below. */
 UInt32 *outputPtr;
 UInt32 temp;

 if(dataFormat == SYSTEM_DF_YUV422I_YUYV)
 {
 wordWidth = (width*2)>>2;
 }
 else
 {
 return SYSTEM_LINK_STATUS_EFAIL;
 }
 inputPtr = inPtr[0];

//OVDJE ČAČKAT

 /* Add code for outputPtr assi2gnment, should start from end. */
 outputPtr = inPtr[0] + height * wordWidth - wordWidth;
 for(rowIdx = 0; rowIdx < height / 2; rowIdx++)
 {
 	for(colIdx = 0; colIdx < wordWidth; colIdx++)
 	{
	/* Add code for swapping values between inputPtr and outputPtr. */
 		temp = *(inputPtr + colIdx);
 		*(inputPtr + colIdx) = *(outputPtr + colIdx);
 		*(outputPtr + colIdx) = temp;
 	}

 inputPtr += (inPitch[0] >> 2);
 /* Add code for decrementing outputPtr. */
 outputPtr -= (inPitch[0] >> 2);
 }
 return SYSTEM_LINK_STATUS_SOK;
}

Int32 Alg_CameraMirrorControl(Alg_CameraMirror_Obj *pAlgHandle,Alg_CameraMirrorControlParams *pControlParams)
{
 /*
 * Any alteration of algorithm behavior
 */
 return SYSTEM_LINK_STATUS_SOK;
}
Int32 Alg_CameraMirrorStop(Alg_CameraMirror_Obj *algHandle)
{
 return SYSTEM_LINK_STATUS_SOK;
}
Int32 Alg_CameraMirrorDelete(Alg_CameraMirror_Obj *algHandle)
{
 free(algHandle);
 return SYSTEM_LINK_STATUS_SOK;
}
