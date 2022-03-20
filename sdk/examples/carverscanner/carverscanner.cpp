/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file carverscanner.cpp
* @brief 
* @details 
* @author Maxwell
* @version 1.0.0
* @date 2018-12-09 22:41:27.492
*
**********************************************************************/
#include "carverscanner.h"
#include <stdio.h>
#include <ctime>
#include <fstream>
#include <filesystem>

using namespace std::filesystem;

const int32_t ConstBytesOfSector	= 512;
const uint64_t ConstRootID			= 0x1000000000000000;
const uint64_t ConstFileCarveID		= 0x2000000000000000;
const uint64_t ConstRawMask			= 0x4000000000000000; 

IScanner* CreateScanner()
{
	return (new CarverScanner());
}

CarverScanner::CarverScanner()
{
	stop_ = true;
	pause_ = false;
	file_count_ = 0;
	delegate_ = nullptr;
}

CarverScanner::~CarverScanner()
{
	carver_container_.clear();
}

int32_t CarverScanner::explore()
{
	return 0;
}

int32_t CarverScanner::advance()
{
	delegate_->Logger("[%s] start", __FUNCTION__);
	int32_t size = 0;
	char szBuffer[4096] = {0x00};
	delegate_->Context(szBuffer, &size);
	info_.assign(szBuffer, size);
	try 
	{
		frjson deviceJson = frjson::parse(info_);
		if (deviceJson.is_null())
			return -1;
		//
		disk_index_ = deviceJson.at("DiskIndex").get<int32_t>();
		sector_size_ = deviceJson.at("BytesPerSector").get<int32_t>();
		sector_size_ = sector_size_ > 0 ? sector_size_ : ConstBytesOfSector;
		offset_ = deviceJson.at("StartingOffset").get<int64_t>();
		device_size_ = deviceJson.at("Size").get<int64_t>();
		//
		FileCarver::WD_SECTOR_SIZE = sector_size_;
	}
	catch (...) 
	{
		delegate_->Logger("exception parse device info");
	}
	
	stop_ = false;
	queue_wait_ = false;
	
	result_future_ = std::async([this] {
		return this->run();
	});
	
	return 0;
}

int32_t CarverScanner::registerCarvers()
{
	std::string strExecutablePath(_pgmptr);
	path executablePath(strExecutablePath);
	std::string executableDir = executablePath.parent_path().string();
	std::string config_path = executableDir + "\\config\\filecarver.json";
	//
	try 
	{
		if (config_setting_.size() > 0)
		{
			config_object_ = frjson::parse(config_setting_);
		}
		else
		{
			std::ifstream is(config_path);
			is >> config_object_;
		}
		//
		if (!config_object_.is_null())
		{
			// first clear
			std::lock_guard<std::mutex> lock(mutex_lock_);
			carver_container_.clear();
			//
			std::string protocol = config_object_.at("protocol").get<std::string>();
			frjson::array_t info_array = config_object_.at("carvers").get<frjson::array_t>();
			for (auto &carver_object : info_array)
			{
				auto carver = std::make_shared<FileCarver>();
				if (carver->setCharacteristics(carver_object) < 0)
					continue;
				carver_container_.emplace_back(carver);
			}
		}

		if (config_setting_.size() > 0)
		{
			std::ofstream o(config_path);
			o << std::setw(4) << config_object_ << std::endl;
 
			config_setting_.clear();
		}
	}
	catch (std::exception& e)
	{
		delegate_->Logger("parse carver config exception: %s", e.what());
	}
	//
	return carver_container_.size();
}

void CarverScanner::stop()
{
	if (stop_)
		return;
	
	stop_ = true;
	if (package_safe_queue_.size() == 0)
	{
		ClusterPackage package;
		package.Option = -1;
		package_safe_queue_.push(package);
	}
	result_future_.wait();
}

void CarverScanner::pause()
{
	if (pause_)
		return;

	pause_ = true;
}

void CarverScanner::resume()
{
	if (!pause_)
		return;
	pause_ = false;
	semap_.signal();
}

const int32_t CarverScanner::filesystem()
{
	return FileSystemCategory::FSC_Raw;
}

bool CarverScanner::availabled(const uint64_t& offset, int32_t option)
{
	return false;
}

int32_t CarverScanner::inject_control(void* data, int32_t& size, int32_t option)
{
	if (option == IC_FileCarverOut)
	{
		std::string config = config_object_.dump();
		if (size < config.size())
			return -1;
		size = config.size();
		memcpy(data, config.c_str(), size);
	}
	else if (option == IC_FileCarverIn)
	{
		config_setting_.assign((char*)data, size);
		//
		registerCarvers();
	}
	//
	return 0;
}

void CarverScanner::set_delegate(ITransferDelegate* delegate)
{
	delegate_ = delegate;
	//
	registerCarvers();
	delegate_->Logger("[%s] init completed", __FUNCTION__);
}

int32_t CarverScanner::read_buffer(char* buffer, int64_t offset, int32_t count)
{
	return 0;
}

int32_t CarverScanner::save_file(char* filePath, int32_t index, int64_t id, int64_t count, int32_t option)
{
	return 0;
}

int32_t CarverScanner::special_read_buffer(void* buffer, int32_t index, int64_t id, int64_t offset, int64_t count, int32_t option)
{
	return 0;
}

void CarverScanner::destroy()
{
	delete this;
}

int64_t last_block_id = -1;

void CarverScanner::write_buffer(const char* buffer, int64_t offset, int32_t count)
{
	if (stop_)
		return;

	ClusterPackage package;
	package.BlockNumber = offset / FileCarver::WD_SECTOR_SIZE;
	memcpy(package.Buffer, buffer, count);
	if (last_block_id == -1)
	{
		last_block_id = package.BlockNumber;
	}
	if (last_block_id != package.BlockNumber)
	{
		last_block_id = package.BlockNumber;
	}
	last_block_id += 8;
				
	package_safe_queue_.push(package);
	
	if (package_safe_queue_.size() > 1024)
	{
		queue_wait_ = true;
		queue_semap_.wait();
	}
}

void CarverScanner::initialize()
{

}

int32_t CarverScanner::run()
{
	while (!stop_)
	{
		auto package = package_safe_queue_.waitPop();
		if (package != nullptr)
		{
			if (package->Option == -1)
			{
				stop_ = true;
				break;
			}
			
			if (!delegate_->Availabled(package->BlockNumber))
				continue;
			
			for (auto& carver : carver_container_)
			{
				if (stop_)
					break;
				
				if (carver->getCarverStatus() == CS_Init)
					carver->analyzeHeader(package);
				
				if (carver->getCarverStatus() == CS_Header)
					carver->analyzeBody(package);
				
				if (carver->getCarverStatus() >= CS_Header)
					carver->analyzeFooter(package);
				
				carver->truncate(package);
				
				if (carver->getCarverStatus() >= CS_Footer)
				{
					serialize(carver);
					carver->initialize();
					break;
				}
			}
		}
		
		if (queue_wait_ && package_safe_queue_.size() < 1024)
		{
			queue_wait_ = false;
			queue_semap_.signal();
		}
		
		if (pause_)
			semap_.wait();
	}
	
	package_safe_queue_.clear();
	if (queue_wait_)
		queue_semap_.signal();
	
	return 0;
}

int32_t CarverScanner::serialize(std::shared_ptr<FileCarver> carver)
{
	if (delegate_ == nullptr)
		return -1;
	
	std::string fileName;
	auto fileInfo = carver->getCarvedFileInfo();
	if (fileInfo->base_name.length() > 0)
	{
		fileName = fileInfo->base_name + "." + carver->getExtension();
	}
	else
	{
		std::string strUuid = MaUtil::GenerateGuid();
		fileName = strUuid.substr(file_count_++%16, 16) + "." + carver->getExtension();
	}
	
	auto info = new BaseInfo();
	info->Id = ConstRawMask + file_count_;
	info->Pid = ConstFileCarveID;
	info->Did = carver->getDeveloperId();
	info->Size = fileInfo->size;
	info->Attribute = 32765;
	info->ScanType = ST_Raw;
	info->FileSystem = FSC_Raw;
	info->Category = 1;
	info->CreateTime = (int64_t)std::time(nullptr);
	info->AccessTime = (int64_t)std::time(nullptr);
	info->ModifyTime = (int64_t)std::time(nullptr);
	
	auto rl = new Runlist();
	rl->Start = fileInfo->start_blockno;
	rl->Number = fileInfo->block_count;
	info->Runlist = rl;
	info->RunlistCategory = RLC_General;
	memcpy(info->Name, fileName.c_str(), fileName.size() > 256 ? 256 : fileName.size());
	
	int64_t len = sizeof(BaseInfo);
	delegate_->Transfer(NotifyOption::NO_FileInfo, info, &len);
	
	return 0;
}
