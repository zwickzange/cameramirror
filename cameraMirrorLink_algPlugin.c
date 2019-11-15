/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * \file CameraMirrorLink_algPlugin.c
 *
 * \brief  This file contains plug in functions for frame copy Link
 *
 * \version 0.0 (Sept 2013) : [NN] First version
 * \version 0.1 (Sept 2013) : [PS] Added code for stats collector
 *
 *******************************************************************************
*/

/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */
#include "CameraMirrorLink_priv.h"
#include <include/link_api/system_common.h>
#include <src/utils_common/include/utils_mem.h>

/**
 *******************************************************************************
 *
 * \brief Implementation of function to init plugins()
 *
 *        This function will be called by AlgorithmLink_initAlgPlugins, so as
 *        register plugins of frame copy algorithm
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_CameraMirror_initPlugin()
{
    AlgorithmLink_FuncTable pluginFunctions;
    UInt32 algId = (UInt32)-1;

    pluginFunctions.AlgorithmLink_AlgPluginCreate =
        AlgorithmLink_CameraMirrorCreate;
    pluginFunctions.AlgorithmLink_AlgPluginProcess =
        AlgorithmLink_CameraMirrorProcess;
    pluginFunctions.AlgorithmLink_AlgPluginControl =
        AlgorithmLink_CameraMirrorControl;
    pluginFunctions.AlgorithmLink_AlgPluginStop =
        AlgorithmLink_CameraMirrorStop;
    pluginFunctions.AlgorithmLink_AlgPluginDelete =
        AlgorithmLink_CameraMirrorDelete;

#ifdef BUILD_DSP
    algId = ALGORITHM_LINK_DSP_ALG_CAMERAMIRROR;
#endif

    AlgorithmLink_registerPlugin(algId, &pluginFunctions);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Create Plugin for color to gray algorithm link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to create time parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_CameraMirrorCreate(void * pObj, void * pCreateParams)
{
    Alg_CameraMirror_Obj              * algHandle;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    System_LinkInfo              prevLinkInfo;
    Int32                        channelId;
    Int32                        numChannelsUsed;
    Int32                        numInputQUsed;
    Int32                        numOutputQUsed;
    UInt32                       prevLinkQueId;

    AlgorithmLink_OutputQueueInfo outputQInfo;
    AlgorithmLink_InputQueueInfo  inputQInfo;

    AlgorithmLink_CameraMirrorObj          * pCameraMirrorObj;
    AlgorithmLink_CameraMirrorCreateParams * pCameraMirrorCreateParams;

    pCameraMirrorCreateParams =
        (AlgorithmLink_CameraMirrorCreateParams *)pCreateParams;

    /*
     * Space for Algorithm specific object gets allocated here.
     * Pointer gets recorded in algorithmParams
     */
    pCameraMirrorObj = (AlgorithmLink_CameraMirrorObj *)
                        malloc(sizeof(AlgorithmLink_CameraMirrorObj));

    UTILS_assert(pCameraMirrorObj!=NULL);

    AlgorithmLink_setAlgorithmParamsObj(pObj, pCameraMirrorObj);

    /*
     * Taking copy of needed create time parameters in local object for future
     * reference.
     */

    memcpy((void*)(&pCameraMirrorObj->outQueParams),
           (void*)(&pCameraMirrorCreateParams->outQueParams),
           sizeof(System_LinkOutQueParams));
    memcpy((void*)(&pCameraMirrorObj->inQueParams),
           (void*)(&pCameraMirrorCreateParams->inQueParams),
           sizeof(System_LinkInQueParams));

    /*
     * Populating parameters corresponding to Q usage of color to gray
     * algorithm link
     */
    numInputQUsed     = 1;
    numOutputQUsed    = 1;

    inputQInfo.qMode  = ALGORITHM_LINK_QUEUEMODE_INPLACE;
    outputQInfo.qMode = ALGORITHM_LINK_QUEUEMODE_INPLACE;

    /*
     * If any output buffer Q gets used in INPLACE manner, then
     * outputQInfo.inQueParams and
     * outputQInfo.inputQId need to be populated appropriately.
     */

    outputQInfo.inputQId = 0;

    memcpy((void*)(&outputQInfo.inQueParams),
           (void*)(&pCameraMirrorCreateParams->inQueParams),
            sizeof(outputQInfo.inQueParams)
        );
    /*
     * Channel info of current link will be obtained from previous link.
     * If any of the properties get changed in the current link, then those
     * values need to be updated accordingly in
     * outputQInfo.queInfo.chInfo[channelId]
     */

    status = System_linkGetInfo(pCameraMirrorCreateParams->inQueParams.prevLinkId,
                                &prevLinkInfo);

    prevLinkQueId = pCameraMirrorCreateParams->inQueParams.prevLinkQueId;

    numChannelsUsed = prevLinkInfo.queInfo[prevLinkQueId].numCh;
    pCameraMirrorObj->numInputChannels = numChannelsUsed;
    outputQInfo.queInfo.numCh = numChannelsUsed;


    for(channelId = 0; channelId < numChannelsUsed; channelId++)
    {
        memcpy((void *)&(outputQInfo.queInfo.chInfo[channelId]),
           (void *)&(prevLinkInfo.queInfo[prevLinkQueId].chInfo[channelId]),
           sizeof(System_LinkChInfo)
          );

    }

    /*
     * Taking a copy of input channel info in the link object for future use
     */

    for(channelId = 0; channelId < numChannelsUsed; channelId++)
    {
        memcpy((void *)&(pCameraMirrorObj->inputChInfo[channelId]),
           (void *)&(prevLinkInfo.queInfo[prevLinkQueId].chInfo[channelId]),
           sizeof(System_LinkChInfo)
          );
    }

    /*
     * Initializations needed for book keeping of buffer handling.
     * Note that this needs to be called only after setting inputQMode and
     * outputQMode.
     */
    AlgorithmLink_queueInfoInit(pObj,
                                numInputQUsed,
                                &inputQInfo,
                                numOutputQUsed,
                                &outputQInfo
                                );

    /*
     * Algorithm creation happens here
     * - Population of create time parameters
     * - Create call for algorithm
     * - Algorithm handle gets recorded inside link object
     */
    algHandle = Alg_CameraMirrorCreate(&pCameraMirrorObj->createParams);
    UTILS_assert(algHandle != NULL);

    pCameraMirrorObj->algHandle = algHandle;

    pCameraMirrorObj->linkStatsInfo = Utils_linkStatsCollectorAllocInst(
        AlgorithmLink_getLinkId(pObj), "ALG_CAMERAMIRROR");
    UTILS_assert(NULL != pCameraMirrorObj->linkStatsInfo);

    pCameraMirrorObj->isFirstFrameRecv = FALSE;

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Process Plugin for color to gray algorithm link
 *
 *        This function executes on the DSP processor. Hence processor gets
 *        locked with execution of the function, until completion. Only a
 *        link with higher priority can pre-empt this function execution.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_CameraMirrorProcess(void * pObj)
{
    AlgorithmLink_CameraMirrorObj * pCameraMirrorObj;
    Alg_CameraMirror_Obj          * algHandle;
    Int32                        inputQId;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       bufId;
    UInt32                       outputQId;
    System_BufferList            inputBufList;
    System_BufferList            inputBufListReturn;
    System_Buffer              * pSysBufferInput;
    System_VideoFrameBuffer    * pSysVideoFrameBufferInput;
    UInt32                       dataFormat;
    UInt32                       bufSize[SYSTEM_MAX_PLANES];
    UInt32                       bufCntr;
    UInt32                       numBuffs;
    UInt32                       channelId;
    System_LinkChInfo          * pInputChInfo;
    Bool                         bufDropFlag = FALSE;
    System_LinkStatistics      * linkStatsInfo;



    pCameraMirrorObj = (AlgorithmLink_CameraMirrorObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);

    linkStatsInfo = pCameraMirrorObj->linkStatsInfo;
    UTILS_assert(NULL != linkStatsInfo);

    algHandle     = pCameraMirrorObj->algHandle;

    /*
     * Getting input buffers from previous link
     */
    System_getLinksFullBuffers(pCameraMirrorObj->inQueParams.prevLinkId,
                               pCameraMirrorObj->inQueParams.prevLinkQueId,
                               &inputBufList);

    Utils_linkStatsCollectorProcessCmd(linkStatsInfo);

    linkStatsInfo->linkStats.newDataCmdCount++;

    if(inputBufList.numBuf)
    {

        if(pCameraMirrorObj->isFirstFrameRecv==FALSE)
        {
            pCameraMirrorObj->isFirstFrameRecv = TRUE;

            Utils_resetLinkStatistics(
                    &linkStatsInfo->linkStats,
                    pCameraMirrorObj->numInputChannels,
                    1);

            Utils_resetLatency(&linkStatsInfo->linkLatency);
            Utils_resetLatency(&linkStatsInfo->srcToLinkLatency);
        }

        for (bufId = 0; bufId < inputBufList.numBuf; bufId++)
        {
          /*
           * Getting free (empty) buffers from pool of output buffers
           */
          outputQId        = 0;

          pSysBufferInput           = inputBufList.buffers[bufId];
          pSysVideoFrameBufferInput = pSysBufferInput->payload;

          channelId = pSysBufferInput->chNum;
          if(channelId < pCameraMirrorObj->numInputChannels)
          {
            linkStatsInfo->linkStats.chStats[channelId].inBufRecvCount++;
          }

          /*
           * Error checks can be done on the input buffer and only later,
           * it can be picked for processing
           */
          if((pSysBufferInput->bufType != SYSTEM_BUFFER_TYPE_VIDEO_FRAME)
             ||
             (channelId >= pCameraMirrorObj->numInputChannels)
            )
          {
            bufDropFlag = TRUE;
            linkStatsInfo->linkStats.inBufErrorCount++;
          }
          else
          {

          bufDropFlag = FALSE;

          /*
           * Call to the algorithm
           */
          dataFormat = System_Link_Ch_Info_Get_Flag_Data_Format(
                        pCameraMirrorObj->inputChInfo[channelId].flags);

          pInputChInfo = &(pCameraMirrorObj->inputChInfo[channelId]);


          switch (dataFormat)
          {
              case SYSTEM_DF_YUV422I_YUYV:
                  numBuffs    = 1;
                  bufSize[0]  = ((pInputChInfo->height)*(pInputChInfo->pitch[0]));
                  break;
              case SYSTEM_DF_YUV420SP_UV:
                  numBuffs    = 1;
                  bufSize[0]  = ((pInputChInfo->height)*(pInputChInfo->pitch[0]));
                  break;
              default:
                  numBuffs    = 1;
                  bufSize[0]  = ((pInputChInfo->height)*(pInputChInfo->pitch[0]));
                  UTILS_assert (0);
                  break;
          }

          pSysBufferInput->linkLocalTimestamp = Utils_getCurGlobalTimeInUsec();

          for(bufCntr = 0; bufCntr < numBuffs; bufCntr++)
          {
            Cache_inv(pSysVideoFrameBufferInput->bufAddr[bufCntr],
                      bufSize[bufCntr],
                      Cache_Type_ALL,
                      TRUE
                     );
          }

          Alg_CameraMirrorProcess(algHandle,
                               (UInt32 **)pSysVideoFrameBufferInput->bufAddr,
                               pInputChInfo->width,
                               pInputChInfo->height,
                               pInputChInfo->pitch,
                               dataFormat
                              );

          for(bufCntr = 0; bufCntr < numBuffs; bufCntr++)
          {
            Cache_wb(pSysVideoFrameBufferInput->bufAddr[bufCntr],
                      bufSize[bufCntr],
                      Cache_Type_ALL,
                      TRUE
                     );
          }

          Utils_updateLatency(&linkStatsInfo->linkLatency,
                              pSysBufferInput->linkLocalTimestamp);
          Utils_updateLatency(&linkStatsInfo->srcToLinkLatency,
                              pSysBufferInput->srcTimestamp);

          linkStatsInfo->linkStats.chStats
                    [channelId].inBufProcessCount++;
          linkStatsInfo->linkStats.chStats
                    [channelId].outBufCount[0]++;

          /*
           * Putting modified input buffer into output full buffer Q
           * Note that this does not mean algorithm has freed the output buffer
           */
          status = AlgorithmLink_putFullOutputBuffer(pObj,
                                                     outputQId,
                                                     pSysBufferInput);

          UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

          /*
           * Informing next link that a new data has peen put for its
           * processing
           */
          System_sendLinkCmd(pCameraMirrorObj->outQueParams.nextLink,
                             SYSTEM_CMD_NEW_DATA,
                             NULL);

          }

          /*
           * Releasing (Free'ing) input buffer, since algorithm does not need
           * it for any future usage.
           */
          inputQId                      = 0;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSysBufferInput;
          AlgorithmLink_releaseInputBuffer(
                                      pObj,
                                      inputQId,
                                      pCameraMirrorObj->inQueParams.prevLinkId,
                                      pCameraMirrorObj->inQueParams.prevLinkQueId,
                                      &inputBufListReturn,
                                      &bufDropFlag);

        }

    }

    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Control Plugin for color to gray algorithm link
 *
 *
 * \param  pObj              [IN] Algorithm link object handle
 * \param  pCreateParams     [IN] Pointer to control parameters
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */

Int32 AlgorithmLink_CameraMirrorControl(void * pObj, void * pControlParams)
{
    AlgorithmLink_CameraMirrorObj     * pCameraMirrorObj;
    AlgorithmLink_ControlParams      * pAlgLinkControlPrm;
    Alg_CameraMirror_Obj              * algHandle;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;

    pCameraMirrorObj = (AlgorithmLink_CameraMirrorObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);
    algHandle     = pCameraMirrorObj->algHandle;

    pAlgLinkControlPrm = (AlgorithmLink_ControlParams *)pControlParams;

    /*
     * There can be other commands to alter the properties of the alg link
     * or properties of the core algorithm.
     * In this simple example, there is just a control command to print
     * statistics and a default call to algorithm control.
     */
    switch(pAlgLinkControlPrm->controlCmd)
    {

        case SYSTEM_CMD_PRINT_STATISTICS:
            AlgorithmLink_CameraMirrorPrintStatistics(pObj, pCameraMirrorObj);
            break;

        default:
            status = Alg_CameraMirrorControl(algHandle,
                                            &(pCameraMirrorObj->controlParams)
                                           );
            break;
    }


    return status;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Stop Plugin for color to gray algorithm link
 *
 *        For this algorithm there is no locking of frames and hence no
 *        flushing of frames. Also there are no any other functionality to be
 *        done at the end of execution of this algorithm. Hence this function
 *        is an empty function for this algorithm.
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_CameraMirrorStop(void * pObj)
{
    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Implementation of Delete Plugin for color to gray algorithm link
 *
 * \param  pObj              [IN] Algorithm link object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_CameraMirrorDelete(void * pObj)
{
    AlgorithmLink_CameraMirrorObj * pCameraMirrorObj;
    Alg_CameraMirror_Obj              * algHandle;
    Int32                        status = SYSTEM_LINK_STATUS_SOK;

    pCameraMirrorObj = (AlgorithmLink_CameraMirrorObj *)
                        AlgorithmLink_getAlgorithmParamsObj(pObj);
    algHandle     = pCameraMirrorObj->algHandle;

    status = Utils_linkStatsCollectorDeAllocInst(pCameraMirrorObj->linkStatsInfo);
    UTILS_assert(status == 0);

    status = Alg_CameraMirrorDelete(algHandle);
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    free(pCameraMirrorObj);

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief Print link statistics
 *
 * \param  pObj                [IN] Algorithm link object handle
 * \param  pCameraMirrorObj       [IN] Color to gray Object handle
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
Int32 AlgorithmLink_CameraMirrorPrintStatistics(void *pObj,
                     AlgorithmLink_CameraMirrorObj *pCameraMirrorObj)
{
    UTILS_assert(NULL != pCameraMirrorObj->linkStatsInfo);

    Utils_printLinkStatistics(&pCameraMirrorObj->linkStatsInfo->linkStats, "ALG_CAMERAMIRROR", TRUE);

    Utils_printLatency("ALG_CAMERAMIRROR",
                       &pCameraMirrorObj->linkStatsInfo->linkLatency,
                       &pCameraMirrorObj->linkStatsInfo->srcToLinkLatency,
                        TRUE
                       );

    return SYSTEM_LINK_STATUS_SOK;
}


/* Nothing beyond this point */
