/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file filecarver.h
* @brief 
* @details 
* @author Maxwell
* @version 1.0.0
* @date 2018-10-09 22:41:27.492
*
**********************************************************************/
#ifndef FILE_CARVER_H
#define FILE_CARVER_H

#include <tuple>
#include <string>
#include <iostream>
#include "../../include/datatype.h"
#include "../../third_party/json.hpp"
#include "../../third_party/mautil.h"

using namespace ma;
using frjson = nlohmann::json;

using namespace std;

typedef enum _AlgorithmType
{
	ALG_CRC32,
} AlgorithmType;

typedef enum _LogicType
{
	LT_None,
	LT_And,
	LT_Or,
	LT_Not
} LogicType;

#pragma pack(push, 1)

typedef struct _NameInfo
{
	uint16_t	offset;
	uint16_t	size;
	uint16_t	padding;
} NameInfo;

typedef struct _CharacterInfo
{
	bool			hex;
	union {
		uint16_t	offset;
		uint16_t	padding;
	} amphibious;
	uint16_t		size;
	uint8_t			character[64];
	_CharacterInfo() {
		hex = false;
		size = 0;
		amphibious.offset = 0;
		amphibious.padding = 0;
		memset(character, 0x00, 64);
	}
} CharacterInfo, *PCharacterInfo;

#pragma pack(pop)

typedef enum _CarverStatus 
{
	CS_Init			= 0,
	CS_Header,
	CS_Body,
	CS_Footer,
	CS_Completed
}CarverStatus;

typedef struct _CarvedFileInfo 
{
	uint64_t	size;
	uint64_t	start_blockno;
	uint64_t	block_count;
	std::string	base_name;
}CarvedFileInfo, *PCarvedFileInfo;

class FileCarver
{
public:
	FileCarver();
	~FileCarver();

	// Public interfaces
	virtual void initialize();

	virtual uint64_t getDeveloperId() const;

	virtual std::string getExtension() const;

	virtual CarverStatus getCarverStatus() const;
	//
	virtual int32_t setCharacteristics(frjson::object_t object);

	virtual std::shared_ptr<CarvedFileInfo> getCarvedFileInfo() const;

	// Interfaces impl by subclass
	virtual int32_t analyzeHeader(std::shared_ptr<ClusterPackage> package);

	virtual int32_t analyzeBody(std::shared_ptr<ClusterPackage> package);

	virtual int32_t analyzeFooter(std::shared_ptr<ClusterPackage> package);

	virtual int32_t truncate(std::shared_ptr<ClusterPackage> package);

public:
	static uint16_t WD_SECTOR_SIZE;

	inline LogicType logicType(std::string& logic);

protected:
	std::string extension_;
	CarverStatus carver_status_;
	std::shared_ptr<CarvedFileInfo> carved_file_info_;
	// configure
	uint64_t developer_id_;
	int64_t truncate_size_;
	std::string algorithm_;
	std::shared_ptr<NameInfo> name_info_;
	std::tuple<LogicType, LogicType, LogicType> logic_tuple_;
	std::vector<std::shared_ptr<CharacterInfo>> header_vector_;
	std::vector<std::shared_ptr<CharacterInfo>> body_vector_;
	std::vector<std::shared_ptr<CharacterInfo>> footer_vector_;
};

#endif // FILE_CARVER_H
