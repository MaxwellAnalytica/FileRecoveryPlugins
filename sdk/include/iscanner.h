/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file iscanner.h
* @brief 
* @details 
* @author Maxwell
* @version 1.0.0
* @date 2018-12-25 12:57:29.892
*
**********************************************************************/
#ifndef SCANNER_INTERFACE_H
#define SCANNER_INTERFACE_H

#ifdef _DLL_EXPORTS
#define EXPORTL_API _declspec(dllexport)
#else
#define EXPORTL_API _declspec(dllimport)
#endif

#include <stdint.h>
/*
*
* @brief Pointer transfer Macro
* @details 
*/
#define ADD_POINTER(type, base, offset) ((type)(((PBYTE)base) + (offset)))
/*
*
* @brief Delegate offer to scanner for exchange data to engine library (scanner <=> engine library) 
* @details 
*/
class ITransferDelegate
{
public:
	/*
	*
	* @brief Random read from current device, offset must be sector alignment
	* @details 
	*/
	virtual int32_t Read(void* buffer, int64_t offset, int32_t count = 512) = 0;
	/*
	*
	* @brief Whether the cluster where the current sector has been allocated, if it has been allocated, skip it. (mainly to improve the scanning speed)
	* @details 
	*/
	virtual bool Availabled(const uint64_t &offset, int32_t option = 0) = 0;
	/*
	*
	* @brief Transfer file info from scanner to engine library when found files
	* @details 
	*/
	virtual int32_t Transfer(int32_t type, void* ptr, int64_t* size) = 0;
	/*
	*
	* @brief Return device context by json including device size, bytes of sector etc.
	* @details 
	*/
	virtual int32_t Context(void* params, int32_t* size) = 0;
	/*
	*
	* @brief Output log according to `printf`
	* @details 
	*/
	virtual int32_t Logger(const char* argv, ...) = 0;
};
/*
*
* @brief Scanner abstract class
* @details Offer common interfaces for file recovery, inherit the class and override virtual functions according to needs
* 	       Not only recover files from disk devices, but also from network file systems, or other like-filesystem morphology
*/
class IScanner
{
public:
	/*
	*
	* @brief Set message transfer delegate
	* @details 
	*/
	virtual void set_delegate(ITransferDelegate* delegate) = 0;
	/*
	*
	* @brief Quick parse and explore lost files according to filesystem protocol without traversing device
	* @details 
	*/
	virtual int32_t explore() = 0;
	/*
	*
	* @brief Traverse device to fetch all datum for making filesystem level analysis, or specified file carve.
	* @details 
	*/
	virtual int32_t advance() = 0;
	/*
	*
	* @brief stop
	* @details 
	*/
	virtual void stop() = 0;
	/*
	*
	* @brief pause
	* @details 
	*/
	virtual void pause() = 0;
	/*
	*
	* @brief resume
	* @details 
	*/
	virtual void resume() = 0;
	/*
	*
	* @brief Filesystem of current scanning device
	* @details 
	*/
	virtual const int32_t filesystem() = 0;
	/*
	*
	* @brief [optional] Whether the cluster where the current sector has been allocated, `option` is for extend
	* @details 
	*/
	virtual bool availabled(const uint64_t &offset, int32_t option = 0) = 0;
	/*
	*
	* @brief [optional] Control parameter inject, for some special needs
	* @details 
	*/
	virtual int32_t inject_control(void* data, int32_t &size, int32_t option = 0) = 0;
	/*
	*
	* @brief [optional] Read data from scanner
	* @details 
	*/
	virtual int32_t read_buffer(char* buffer, int64_t offset, int32_t count = 4096) = 0;
	/*
	*
	* @brief [optional] Data written from engine library, scanner is callee, used to advance scan model.
	* @details 
	*/
	virtual void write_buffer(const char* buffer, int64_t offset, int32_t count = 4096) = 0;
	/*
	*
	* @brief [optional] Save file according to info offered by engine library in some special case that only scanner can do it
	* @details 
	*/
	virtual int32_t save_file(char* filePath, int32_t index, int64_t id, int64_t count, int32_t option = 0) = 0;
	/*
	*
	* @brief [optional] Read data according to info offered by engine library in some special case that only scanner can do it
	* @details 
	*/
	virtual int32_t special_read_buffer(void* buffer, int32_t index, int64_t id, int64_t offset, int64_t count, int32_t option = 0) = 0;
	/*
	*
	* @brief Resource release
	* @details 
	*/
	virtual void destroy() = 0;

protected:
	/*
	*
	* @brief [optional] thread call back funtion
	* @details 
	*/
	virtual int32_t run() = 0;
};
/*
*
* @brief  Create scanner object instance
* @details 
*/
extern "C" EXPORTL_API IScanner *CreateScanner();

#endif /* SCANNER_INTERFACE_H */ 