/********************************************************************
 *A scalable and high-performance platform for R.
 *Copyright (C) [2013] Hewlett-Packard Development Company, L.P.

 *This program is free software; you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation; either version 2 of the License, or (at
 *your option) any later version.

 *This program is distributed in the hope that it will be useful, but
 *WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *General Public License for more details.  You should have received a
 *copy of the GNU General Public License along with this program; if
 *not, write to the Free Software Foundation, Inc., 59 Temple Place,
 *Suite 330, Boston, MA 02111-1307 USA
 ********************************************************************/

/**
 * Main class for Executor Scheduler
 */

#ifndef _TASK_SCHEDULER_
#define _TASK_SCHEDULER_

/*#include <stdio.h>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>*/

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "common.h"
#include "dLogger.h"
#include "shared.pb.h"
#include "timer.h"

namespace presto {

class ExecutorPool;
class PrestoWorker;

struct ExecSplit {
  boost::unordered_set<int> executors;
  std::string name;
  size_t size;
};

struct SplitUpdate {
  std::string name;
  size_t size;
  int executor;
};

struct ExecStat {
   int64_t persist_load;
   int64_t exec_load;
   size_t mem_used;
};

class TaskScheduler {
 public:
  TaskScheduler(PrestoWorker* pw_,ExecutorPool* ep_, boost::unordered_set<std::string> *shmem_arrays_,
                boost::timed_mutex *shmem_arrays_mutex_, int nExecutors):
                worker(pw_),executorpool(ep_), shmem_arrays(shmem_arrays_),
                shmem_arrays_mutex(shmem_arrays_mutex_) {
    for(int i=0; i < nExecutors; i++) {
        AddExecutor(i);
    }
  }

  ~TaskScheduler() {}

  void foreachcomplete(bool status);
  void StageUpdatedPartition(const std::string& split_name, size_t size, int executor_id);
  void ValidatePartitions(const std::vector<NewArg>& task_args, int executor_id, uint64_t taskid);
  int64_t AddParentTask(const std::vector<NewArg>& task_args, int64_t parenttaskid);
  void DeleteSplit(const std::string& splitname);

protected:
  // Returns the executor on which the task should be executed.
  int64_t GetBestExecutor(const std::vector<NewArg>& partitions);
  int64_t ExecutorToPersistFrom(const std::string& split_name);
  bool IsSplitAvailable(const std::string& split_name, int executor_id=-1); //If executor_id is -1, then persist to worker.
  bool IsBeingPersisted(const std::string& split_name);

  int GetDeterministicExecutor(int32_t split_id);
  
 private:
  void AddExecutor(int id);
  boost::unordered_map<std::string, ExecSplit*> executor_splits; //map<name, split_info>
  boost::unordered_set<std::string> *shmem_arrays;
  boost::unordered_map<int, ExecStat*> executor_stat;  //which executor has how much memory
  boost::unordered_map<uint64_t, int> parent_tasks;   //map<parent_task, exec_id>//to keep track of executor on which execution will happen.

  // Metadata for fetch/newtransfer
  //boost::unordered_set<std::string> fetched_partitions_; //All Partitions fetched into a worker from another worker
  //boost::unordered_set<std::string> persisted_partitions_; //All Partitions persisted on worker

  //Stage metadata
  boost::unordered_set<SplitUpdate*> updated_splits;

  //Process Communication
  boost::recursive_mutex parent_mutex;
  boost::recursive_mutex metadata_mutex;
  boost::timed_mutex *shmem_arrays_mutex;

  ExecutorPool* executorpool;
  PrestoWorker* worker;
};

}  // namespace presto

#endif
