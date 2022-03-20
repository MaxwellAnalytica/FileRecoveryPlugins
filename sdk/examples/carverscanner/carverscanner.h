/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file carverscanner.h
* @brief 
* @details 
* @author Maxwell
* @version 1.0.0
* @date 2018-12-09 22:41:27.492
*
**********************************************************************/
#ifndef CARVER_SCANNER_H
#define CARVER_SCANNER_H

#include <future>
#include "filecarver.h"
#include "../../include/iscanner.h"
#include "../../third_party/safequeue.h"

class FileCarver;

class CarverScanner : public IScanner
{
public:
	CarverScanner();
	virtual ~CarverScanner();

	virtual void set_delegate(ITransferDelegate* delegate);

	virtual int32_t explore();

	virtual int32_t advance();

	virtual void stop();

	virtual void pause();

	virtual void resume();

	virtual const int32_t filesystem();

	virtual bool availabled(const uint64_t& offset, int32_t option = 0);

	virtual int32_t inject_control(void* data, int32_t &size, int32_t option = 0);

	virtual void write_buffer(const char* buffer, int64_t offset, int32_t count = 4096);

	virtual int32_t read_buffer(char* buffer, int64_t offset, int32_t count = 4096);

	virtual int32_t save_file(char* filePath, int32_t index, int64_t id, int64_t count, int32_t option = 0);

	virtual int32_t special_read_buffer(void* buffer, int32_t index, int64_t id, int64_t offset, int64_t count, int32_t option = 0);

	virtual void destroy();


protected:
	virtual int32_t run();

	void initialize();

	int32_t registerCarvers();

	int32_t serialize(std::shared_ptr<FileCarver> carver);

private:
	bool stop_;
	bool pause_;
	std::string info_;
	int64_t offset_;
	int32_t disk_index_;
	int32_t sector_size_;
	int64_t device_size_;
	int64_t file_count_;
	ma::Semaphore semap_;
	bool queue_wait_;
	ma::Semaphore queue_semap_;
	//
	frjson config_object_;
	//
	ITransferDelegate* delegate_;
	//
	std::mutex mutex_lock_;
	std::string config_setting_;
	std::vector<std::shared_ptr<FileCarver> > carver_container_;
	//
	std::future<int32_t> result_future_;
	ma::Safequeue<ClusterPackage> package_safe_queue_;
};

#endif // CARVER_SCANNER_H