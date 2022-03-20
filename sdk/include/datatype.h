/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file datatype.h
* @brief 
* @details 
* @author Maxwell
* @version 1.0.0
* @date 2018-12-25 12:57:29.892
*
**********************************************************************/
#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <stdint.h>
#include <string.h>

#define WD_BLOCK_SIZE 4096

/*
*
* @brief:	return value 
* @details
*/
enum 
{
	WFC_Success		= 0,		/* success */
	WFC_Error		= -1,		/* error */
	WFC_Fatal		= -2,		/* fatal */
	WFC_Abort		= -3,		/* abort */
	WFC_Busy		= -4,		/* busy */
};
/*
*
* @brief:	filesystem type
* @details: apply to corresponding scanner
* 
*/
enum FileSystemCategory 
{
	FSC_None,					/* None */
	FSC_Fat16,					/* FAT16 */
	FSC_Fat32,					/* FAT32 */
	FSC_ExFat,					/* exFAT */
	FSC_Ntfs,					/* NTFS */
	FSC_Hfsp,					/* MacOS Extends */
	FSC_Apfs,					/* Apple filesystem */
	FSC_Raw,					/* File carver */
	FSC_RecycleBin,				/* Recycle bins */
	FSC_Advance,				/* Advance file carver */
};
/*
*
* @brief:	data run list types
* @details 
*/
enum 
{
	RLC_Unused		= 0,		/* no use */
	RLC_General,				/* general data run list */
	RLC_Inside,					/* resident file that data in a sector or cluster */
};
/*
*
* @brief:	scan types
* @details
*/
enum 
{
	ST_Explore = 0,	/* explore files of volume */
	ST_Delete,		/* quick scan, analyze the structure base on filesystem */
	ST_Format,		/* format scan, sequential analyze sector reference filesystem */
	ST_Raw,			/* general file carve */
	ST_Copy			/* file copy */
};
/*
*
* @brief:	Inject control
* @details: direction: Out: core -> UI；In: core <- UI
*/
typedef enum _InjectControlOption
{
	IC_ActuatorOut			= 0x0000,
	IC_ActuatorIn			= 0x0001,
	IC_FileCarverOut		= 0x0002,
	IC_FileCarverIn			= 0x0003,
	IC_SettingOut			= 0x0005,
	IC_SettingIn			= 0x0006,
	IC_CheckCertificate		= 0x000A,
	IC_EFSCertificate		= 0x000B,
	IC_EFSpassword			= 0x000C,
	IC_DiscardCertificate	= 0x000D,
	IC_ProtectionStatus		= 0x000E,
	IC_CreateCertificate	= 0x000F,
	IC_EncryptedFlag		= 0x4000,
}InjectControlOption;
/*
*
* @brief:	Notice or Message types
* @details 
*/
typedef enum _NotifyOption
{
	NO_Unknown = 0,
	NO_Start,
	NO_Stop,
	NO_Pause,
	NO_Resume,
	NO_Milestone,
	NO_Completed,
	NO_FileInfo,
	NO_Password,
	NO_Progress,
	NO_Disconnect,
	NO_BadCluster,
}NotifyOption;

#pragma pack(1)
/*
*
* @brief:	data run list, measured in sectors
* @details
*/
struct Runlist {
	uint64_t	Offset;		/* file offset at the runlist, measured in bytes*/
	uint64_t	Start;		/* start sectors of runlist */
	uint64_t	Number;		/* number sectors of runlist */
	Runlist*	Next;		/* address of next runlist */
	Runlist() {
		Offset	= 0;
		Start	= 0;
		Number	= 0;
		Next	= nullptr;
	}
};
/*
*
* @brief:  data package
* @details
*/
typedef struct _ClusterPackage
{
	int32_t			Option;					/* package type */
	uint64_t		BlockNumber;			/* start sector of buffer */
	char			Buffer[WD_BLOCK_SIZE];	/* data */
	_ClusterPackage() {
		Option		= 0;
		BlockNumber = 0;
		memset(Buffer, 0x00, WD_BLOCK_SIZE);
	}
}ClusterPackage, *PClusterPackage;
/*
*
* @brief:	file information
* @details 
*/
struct BaseInfo {
	uint64_t	Id;					/* file unique id */
	uint64_t	Pid;				/* parent id */
	uint64_t	Did;				/* developer id */
	uint64_t	Size;				/* file size，directory is 0 */
	uint16_t	Flag;				/* delete flag，0 for not delete，1 for deleted */
	Runlist*	Runlist;			/* data run list, file storage address on disk */
	uint32_t	Category;			/* file type，0 for directory, 1 for file */
	uint32_t	ScanType;			/* file from which scan type */
	uint32_t	Attribute;			/* file attribute */
	uint32_t	FileSystem;			/* file from which filesystem */
	uint32_t	RunlistCategory;	/* 0 for no runlist; 1 general runlist; 2 for resident runlist*/
	uint64_t	CreateTime;			/* file create datetime */
	uint64_t	AccessTime;			/* file access datetime */
	uint64_t	ModifyTime;			/* file modify datetime */
	uint8_t		Name[256];			/* file name, limit 256 bytes */
	BaseInfo() {
		Id = 0;
		Pid = 0;
		Did = 0;
		Size = 0;
		Flag = 0;
		Runlist = nullptr;
		Category = 0;
		ScanType = 0;
		Attribute = 0;
		FileSystem = 0;
		RunlistCategory = 0;
		CreateTime = 0;
		AccessTime = 0;
		ModifyTime = 0;
		memset(Name, 0x00, 256);
	}
};
/*
*
* @brief: FAT file information, inherit `BaseInfo`
* @details 
*/
struct FatFileInfo : BaseInfo {
	uint64_t	MaxClusterId;
	uint64_t	RootSectorId;
	uint32_t	SectorOfCluster;
	uint32_t	SectorSize;
};
/*
*
* @brief: EXFAT file information, inherit `BaseInfo`
* @details 
*/
struct ExFatFileInfo : BaseInfo {
	uint64_t	MaxClusterId;
	uint64_t	RootSectorId;
	uint32_t	SectorOfCluster;
	uint32_t	SectorSize;
};
/*
*
* @brief: NTFS file information, inherit `BaseInfo`
* @details 
*/
struct NtfsFileInfo : BaseInfo {
	uint64_t	MaxClusterId;
	uint64_t	RootSectorId;
	uint32_t	SectorOfCluster;
	uint32_t	SectorSize;
};
/*
*
* @brief: HFSP file information, inherit `BaseInfo`
* @details 
*/
struct HfspFileInfo : BaseInfo {
	uint64_t	MaxClusterId;
	uint64_t	RootSectorId;
	uint32_t	SectorOfCluster;
	uint32_t	SectorSize;
};
/*
*
* @brief: APFS file information, inherit `BaseInfo`
* @details 
*/
struct ApfsFileInfo : BaseInfo {
	uint64_t	MaxClusterId;
	uint64_t	RootSectorId;
	uint32_t	SectorOfCluster;
	uint32_t	SectorSize;
};

#pragma pack()

#endif /* DATA_TYPE_H */ 