/*
 * Logging.cpp
 *
 *  Created on: Dec 9, 2016
 *      Author: duclv
 */

#include "Logging.h"
#include "Util.h"
#include "Table.h"
#include <stdio.h>

namespace std {

Logging::Logging() {
	privateLogBuffer = new vector<logging>();
}

Logging::~Logging() {
	if (privateLogBuffer != NULL) {
		delete privateLogBuffer;
	}
}

vector<Logging::logging>* Logging::publicLogBuffer = new vector<Logging::logging>();


void Logging::saveCheckpoint(Table* table) {
	// create folder log
	Util::createFolder(this->logPath);

	// write content
	string fileName = this->logPath + "/checkpoint_" + to_string(Util::currentMilisecond());
	if (table != NULL) {
		vector<string>* content = new vector<string>();
		content->push_back("Checkpoint start " + to_string(Util::currentMilisecond()));
		content->push_back(table->saveToDisk(this->logPath));
		content->push_back("Checkpoint end " + to_string(Util::currentMilisecond()));
		Util::saveToDisk(content, fileName);
	}
}

void Logging::redoLogUpdate(size_t txIdx, LOG_TX_ACTION txAction){
	logging newLog;
	newLog.txIdx = txIdx;
	newLog.txAction = txAction;
	privateLogBuffer->push_back(newLog);
}

void Logging::redoLogAdd(size_t txIdx, string colName, LOG_OBJECT objType, vector<string> logContent) {
	logging newLog;
	newLog.txIdx = txIdx;
	newLog.colName = colName;
	newLog.objType = objType;
	newLog.logContent = logContent;
	privateLogBuffer->push_back(newLog);
}

void Logging::redoLogPublicMerge() {
	// save this private log buffer to public buffer
	publicLogBuffer->insert(publicLogBuffer->end(), this->privateLogBuffer->begin(), this->privateLogBuffer->end());
}

void Logging::redoLogSave() {
	string logFileName = this->logPath + "/redo_log_" + to_string(Util::currentMilisecond());
	vector<string>* content = new vector<string>();
	for (size_t i = 0; i < publicLogBuffer->size(); i++) {
		logging log = publicLogBuffer->at(i);
		switch (log.txAction) {
			case TX_START: {
				content->push_back(to_string(log.txIdx) + "|start");
				break;
			}
			case TX_COMMIT: {
				content->push_back(to_string(log.txIdx) + "|commit");
				break;
			}
			case TX_END: {
				content->push_back(to_string(log.txIdx) + "|end");
				break;
			}
		}
		string objValue = to_string(log.txIdx) + "|";
		if (log.colName != "") {
			objValue += log.colName + "|";
		}
		switch (log.objType) {
			case INSERT:
				objValue += "insert|";
				break;
			case DELTA_SPACE:
				objValue += "delta_space|";
				break;
			case VERSION_VECVALUE:
				objValue += "version_vec_value|";
				break;
			case HASHTABLE:
				objValue += "hashtable|";
				break;
			case VERSION_COLUMN:
				objValue += "version_column|";
				break;
		}
		if (log.logContent.size() > 0) {
			string tmpValue = "";
			for (size_t i = 0; i < log.logContent.size(); i++) {
				string value = log.logContent.at(i);
				tmpValue += value + "|";
			}
			objValue += tmpValue;
			content->push_back(objValue);
		}
	}
	if (publicLogBuffer->size() > 0) {
		cout << "Start log save" << endl;
		// save to disk
		Util::saveToDisk(content, logFileName);
		publicLogBuffer->resize(0);
		cout << "End log save" << endl;
	}
}

void Logging::restore(Table* table) {
	// get latest checkpoint
	string latestCkpt = Util::getLatestFile(logPath, "checkpoint");
	if (latestCkpt != "") {
		// restore from checkpoint
		vector<string>* content = new vector<string>();
		Util::readFromDisk(content, latestCkpt);
		if (content->size() >= 3) {
			string tableFile = content->at(1);
			table->restore(tableFile);
		}
		delete content;
		// get redo log from checkpoint
		long ckptTime = stol(latestCkpt.substr(latestCkpt.find("checkpoint") + 1));
		vector<string> allRedoLogs = Util::getNewestFiles(logPath, "redo_log", ckptTime);
		// sort by oldest to newest file name
		std::sort(allRedoLogs.begin(), allRedoLogs.end());
		// restore redo log
		for (string redoFile : allRedoLogs) {
			vector<string>* redoContent = new vector<string>();
			Util::readFromDisk(redoContent, redoFile);
			vector<string>* deltaSpaceLog = new vector<string>();
			vector<string>* versionVecValueLog = new vector<string>();
			vector<string>* insertLog = new vector<string>();
			vector<string>* versionColumnLog = new vector<string>();
			vector<string>* hashtableLog = new vector<string>();

			for (size_t i = 0; i < redoContent->size(); i++) {
				string log = redoContent->at(i);
				string colName = "";
				if (log.find("start") != string::npos) {

				}
				if (log.find("delta_space") != string::npos) {
					string logContent = log.substr(log.find("delta_space")+1);
					Util::parseContentToVector(deltaSpaceLog, logContent, "|");
				}
				else if (log.find("insert") != string::npos) {
					string logContent = log.substr(log.find("insert")+1);
					Util::parseContentToVector(insertLog, logContent, "|");
				}
				else if (log.find("version_vec_value") != string::npos) {
					string logContent = log.substr(log.find("version_vec_value")+1);
					Util::parseContentToVector(versionVecValueLog, logContent, "|");
				}
				else if (log.find("version_column") != string::npos) {
					string logContent = log.substr(log.find("version_column")+1);
					Util::parseContentToVector(versionColumnLog, logContent, "|");
				}
				else if (log.find("hashtable") != string::npos) {
					string logContent = log.substr(log.find("hashtable")+1);
					Util::parseContentToVector(hashtableLog, logContent, "|");
				}
				if (log.find("end") != string::npos) {
					// restore

				}
			}
		}
	}
}

} /* namespace std */
