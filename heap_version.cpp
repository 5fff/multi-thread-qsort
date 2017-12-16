//Ruicheng Xu    rx8qd@mst.edu
#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <fstream>
#include <vector>
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std;

mutex job_queue_mtx;
const unsigned MAX_N_THREAD=10;
bool njp_flag[MAX_N_THREAD];//new job possible flag
struct job
{
	int l,r;//left and right
};

bool any_njp_flag()//return true if any flag is true
{
	//cout<<"any_njp_flag() is called"<<endl;
	for(unsigned i=0;i<MAX_N_THREAD;i++)if(njp_flag[i])return true;
	return false;
}

void qsort_worker(int t_id, int* a, deque<job> &job_queue)
{
	//cout<<"qsort_worker id = "<<t_id<<endl;
	
	unique_lock<mutex> job_queue_lock(job_queue_mtx, defer_lock);
	int start, finish, i;
	int cap, pivot_val;
	job local_job, outsource_job;
	
	
	while(true)
	{
		
		
		//critical section, get job
		//cout<<"qsort_worker id = "<<t_id<<"requiring cc job queue"<<endl;
		if(!job_queue_lock.try_lock())continue;
		//cout<<"qsort_worker id = "<<t_id<<"requiring cc successful"<<endl;
		if(job_queue.empty())
		{
			if(any_njp_flag())
			{
				//cout<<"qsort_worker id = "<<t_id<<"queue ampty, njp true, continue"<<endl;
				job_queue_lock.unlock();
				continue;
				//if job queue is empty but new job is possible, 
				//then loop around and wait for new job
			}
			else
			{
				//cout<<"qsort_worker id = "<<t_id<<"queue ampty, njp false, unlock"<<endl;
				job_queue_lock.unlock();
				break;
			}
		}
		
		njp_flag[t_id]=true; //when you took a job away from list, you dont know if 
		//you will out source a new job or not
		
		
		local_job=job_queue.front();
		//cout<<"qsort_worker id = "<<t_id<<"accessing job queue size="<< job_queue.size()<<endl;
		job_queue.pop_front();
		
		//cout<<"qsort_worker id = "<<t_id<<"unlock"<<endl;
		job_queue_lock.unlock();
		//=====================
		
		
		
		while(true)
		{
			start=local_job.l;
			finish=local_job.r;
			cap=start;
			pivot_val= a[finish];
			
			//cout<<"array content: ";
			//for(int j=start;j<=finish;j++)cout<<a[j]<<" ";
			//cout<<endl;
			
			for(i = start; i<=finish;i++)
			{
				if(a[i]<=pivot_val)
				{
					if(i!=cap)swap(a[i],a[cap]); //_ _ _ _ _
					cap++;
				}
			}
			
			if(cap-start>=3)//if left sub array has job to do, then do the left, and outsource the right sub if any
			{
				if(cap<finish)//outsource first
				{
					//outsource!!!!!!!!!!!!!!!!!!!!!!!! 
					outsource_job.l=cap;
					outsource_job.r=finish;
					//-----------------------
					//cout<<"qsort_worker id = "<<t_id<<"outsourcing job"<<endl;
					job_queue_lock.lock();
					job_queue.push_back(outsource_job);
					job_queue_lock.unlock();
					//cout<<"qsort_worker id = "<<t_id<<"outsourcing job successful"<<endl;
					//--------------------------
				}
				//then give itself new job
				local_job.l=start;
				local_job.r=cap-2;
				
				
			}
			else 
			{
				if(cap<finish)//no job on the left, then do the right side itself
				{
					local_job.l=cap;
					local_job.r=finish;
				}else{
					njp_flag[t_id]=false;//this thread wont outsource anyjob
					break; //no job left, then go to parent loop to find new jobs from queue;
				}
			}
			
			
		}
	
	
	
	}
	
	
	
	
}

int main()
{
	unsigned max_concurrency = std::thread::hardware_concurrency();
	chrono::high_resolution_clock::time_point t1,t2;
	chrono::duration<double> time_span;
	deque<job> job_queue;
	
	ifstream file_read("data");
    ofstream file_write("out");
    int n_thread;
	int n;
    cout<<"# of elements to read:";
    cin>>n;
    int* a = new int[n];
    for(int i=0;i<n;i++)file_read>>a[i];
    cout<<"read finished!"<<endl;
	cout<<"Recommend # of threads = "<<max_concurrency<<endl;
	cout<<"Enter # of threads: ";
	cin>>n_thread;
	
	
	t1 = chrono::high_resolution_clock::now();//==================
	
	
	for(auto&& i:njp_flag)i=false;
	job first_job={0,(n-1)};
	job_queue.push_front(first_job);
	vector<thread> qsort_worker_threads;
	
	
	
	for(int i=0;i<n_thread;i++)
	{
		qsort_worker_threads.push_back(std::thread(qsort_worker, i, (&a[0]), ref(job_queue)));
	}
	for(auto &t : qsort_worker_threads)
	{
		t.join();
	}
	
	t2 = chrono::high_resolution_clock::now();
	time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	
	cout<<"Time used "<<time_span.count()<<" seconds"<<endl;
	for(int i=0;i<n;i++)file_write<<a[i]<<" ";
    cout<<"write finished!"<<endl;
	
	delete [] a;
	
    return 0;
	
}
//g++ -pthread -std=c++11 -O3 main.cpp








































