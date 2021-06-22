#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>
using namespace std;

int cancel[3];
int canthread;
struct Data
{ //data structure to store row and col
	int row;
	int col;
	int thr_no;
	Data(int _row = -1, int _col = -1, int _thr = -1)
	{
		row = _row;
		col = _col;
		thr_no = _thr;
	}
};
int output_final[11];
struct Err_log
{ //data structure to store errors
	int row;
	int col;
	int val;
	char status;
	char typee;
	pthread_t pid_thr;

	Err_log(int _row = -1, int _col = -1, int _val = -1, char _status = 'n', char _typee = 'n', pthread_t _pid_thr = 0)
	{
		row = _row;
		col = _col;
		val = _val;
		status = _status;
		typee = _typee;
		pid_thr = _pid_thr;
	}
	void print_all()
	{
		string k1, k2;
		if (status == 'i')
		{
			k1 = "Invalid";
		}
		else if (status == 'r')
		{
			k1 = "Repeated";
		}
		else if (status == 's')
		{
			k1 = "Solved";
		}
		if (typee == 'r')
		{
			k2 = "Row";
		}
		else if (typee == 'c')
		{
			k2 = "Column";
		}
		else if (typee == 'g')
		{
			k2 = "Grid";
		}
		cout << "Value : " << val << ", Row : " << row << ", Column : " << col << " Error type : " << k1 << " by " << k2 << " thread... ID= " << pid_thr << endl;
	}
};
struct Thrd_log
{ //data structure to cancellation records
	pthread_t pid_thra;
	pthread_t pid_thrb;

	Thrd_log(pthread_t _pid_thra = 0, pthread_t _pid_thrb = 0)
	{
		pid_thra = _pid_thra;
		pid_thrb = _pid_thrb;
	}
	void print_all()
	{

		cout << " Thread : " << pid_thra << " cancelled "
			 << " thread : " << pid_thrb << endl;
	}
};

ostream &operator<<(ostream &o, Data d)
{
	o << d.row << "x" << d.col;
}

int s[9][9]; 
pthread_mutex_t mutex1;
pthread_t th_id1 = 0, th_id2 = 0, th_id3[9] = {0};
Data d[9];				   //data structure
vector<Err_log> err_log;   //vector to store wrong indexs in form of struct errlog
vector<Thrd_log> thrd_log; //vector to store wrong indexs in form of struct errlog

int give_pos_thd(int row, int col)
{
	if (col <= 2 && col >= 0)
	{
		if (row <= 2 && row >= 0)
		{
			return 0;
		}
		else if (row > 2 && row <= 5)
		{
			return 3;
		}
		else if (row > 5 && row <= 8)
		{
			return 6;
		}
	}
	else if (col > 2 && col <= 5)
	{
		if (row <= 2 && row >= 0)
		{
			return 1;
		}
		else if (row > 2 && row <= 5)
		{
			return 4;
		}
		else if (row > 5 && row <= 8)
		{
			return 7;
		}
	}
	else if (col > 5 && col <= 8)
	{
		if (row <= 2 && row >= 0)
		{
			return 2;
		}
		else if (row > 2 && row <= 5)
		{
			return 5;
		}
		else if (row > 5 && row <= 8)
		{
			return 8;
		}
	}
}

void handle_threads(int row, int col, int t_typ, bool x);
int correct(int row, int col, int t_typ)
{
	int output = 0;
	int all_agree = -1;
	int maj_agree = -1;
	int partial_agreement = 0;
	//handle_threads(row, col, t_typ, true);
	bool colcheck[9] = {0}, rowcheck[9] = {0}, gridcheck[9] = {0};
	for (int i = 0; i < 9; i++)
	{ //checking in the column
		if (s[i][col] > 0 && s[i][col] < 10)
		{
			colcheck[s[i][col] - 1] = true;
		}
		//checking in the row
		if (s[row][i] > 0 && s[row][i] < 10)
		{
			rowcheck[s[row][i] - 1] = true;
		}
	}
	for (int i = row; i < row + 3; i++)
	{
		for (int j = col; j < col + 3; j++)
		{
			if (s[i][j] > 0 && s[i][j] < 10)
			{
				gridcheck[s[i][j] - 1] = true;
			}
		}
	}
	for (int i = 0; i < 9; i++)
	{
		if (colcheck[i] == false && rowcheck[i] == false && gridcheck[i] == false)
		{
			all_agree = i + 1;
			break;
		}
		if (colcheck[i] == false && rowcheck[i] == false)
		{
			maj_agree = i + 1;
		}
		else if (colcheck[i] == false || rowcheck[i] == false || gridcheck[i] == false)
		{
			partial_agreement = i + 1;
		}
	}
	if (all_agree != -1)
	{
		s[row][col] = all_agree;
		output = all_agree;
	}
	else if (maj_agree != -1)
	{
		s[row][col] = maj_agree;
		output = maj_agree;
	}
	else
	{
		s[row][col] = partial_agreement;
		output = partial_agreement;
	}
	//handle_threads(row, col, t_typ, false);
	return output;
}

void *colthread(void *)
{
	int x;
	int k_res;
	cancel[0] = 1;
	int output_f = 1;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &x);

	for (int col = 0; col < 9; col++)
	{
		bool check[9] = {false};
		for (int i = 0; i < 9; i++)
		{
			if (s[i][col] < 1 || s[i][col] > 9)
			{
				pthread_mutex_lock(&mutex1);
				Err_log tempp(i + 1, col + 1, s[i][col], 'i', 'c', pthread_self());
				err_log.push_back(tempp);
				k_res = correct(i, col, 1);
				Err_log temppa(i + 1, col + 1, k_res, 's', 'c', pthread_self());
				err_log.push_back(temppa);
				pthread_cancel(th_id2);
				cancel[0] = 0;
				pthread_exit(0);
				pthread_mutex_unlock(&mutex1);

				//output_f = 0;
			}
			else if (check[s[i][col] - 1] == true)
			{ //dublicate value found
				pthread_mutex_lock(&mutex1);
				Err_log tempp(i + 1, col + 1, s[i][col], 'r', 'c', pthread_self());
				err_log.push_back(tempp);
				k_res = correct(i, col, 1);
				Err_log temppa(i + 1, col + 1, k_res, 's', 'c', pthread_self());
				err_log.push_back(temppa);
				pthread_cancel(th_id2);
				cancel[0] = 0;
				pthread_exit(0);
				pthread_mutex_unlock(&mutex1);
				//output_f = 0;
			}
			else
			{
				check[s[i][col] - 1] = true;
			}
		}
	}

	pthread_mutex_lock(&mutex1);
	output_final[0] = output_f;
	pthread_mutex_unlock(&mutex1);
}

void *rowthread(void *)
{
	int k_res;
	int output_f = 1;
	int x;
	cancel[1] = 1;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &x);

	for (int row = 0; row < 9; row++)
	{
		bool check[9] = {false};
		for (int i = 0; i < 9; i++)
		{
			if (s[row][i] < 1 || s[row][i] > 9)
			{
				pthread_mutex_lock(&mutex1);
				Err_log tempp(row + 1, i + 1, s[row][i], 'i', 'r', pthread_self());
				err_log.push_back(tempp);
				k_res = correct(row, i, 0);
				Err_log temppa(row + 1, i + 1, k_res, 's', 'r', pthread_self());
				err_log.push_back(temppa);
				pthread_cancel(th_id1);
				cancel[1] = 0;
				pthread_exit(0);
				pthread_mutex_unlock(&mutex1);
				output_f = 1;
			}
			else if (check[s[row][i] - 1] == true)
			{
				pthread_mutex_lock(&mutex1);
				Err_log tempp(row + 1, i + 1, s[row][i], 'r', 'r', pthread_self());
				err_log.push_back(tempp);
				k_res = correct(row, i, 0);
				Err_log temppa(row + 1, i + 1, k_res, 's', 'r', pthread_self());
				err_log.push_back(temppa);
				pthread_cancel(th_id1);
				cancel[1] = 0;
				pthread_exit(0);
				pthread_mutex_unlock(&mutex1);
				//output_f = 1;
			}
			else
			{
				check[s[row][i] - 1] = true;
			}
		}
	}

	pthread_mutex_lock(&mutex1);
	output_final[1] = output_f;
	pthread_mutex_unlock(&mutex1);
}

void *gridthread(void *argv)
{
	int k_res;
	int output_f = 1;
	int x;
	cancel[2] = 1;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &x);
	// give argument row and col, and call checkGrid function with argument
	Data *d = (Data *)argv;

	bool check[9] = {false};
	for (int i = d->row; i < d->row + 3; i++)
	{
		for (int j = d->col; j < d->col + 3; j++)
		{
			if (s[i][j] < 1 || s[i][j] > 9)
			{
				pthread_mutex_lock(&mutex1);
				Err_log tempp(i + 1, j + 1, s[i][j], 'i', 'g', pthread_self());
				err_log.push_back(tempp);
				k_res = correct(i, j, 2);
				Err_log temppa(i + 1, j + 1, k_res, 's', 'g', pthread_self());
				err_log.push_back(temppa);
				cancel[2] = 0;
				canthread = d->thr_no;
				pthread_mutex_unlock(&mutex1);
				//output_f = 1;
			}
			else if (check[s[i][j] - 1] == true)
			{
				pthread_mutex_lock(&mutex1);
				Err_log tempp(i + 1, j + 1, s[i][j], 'r', 'g', pthread_self());
				err_log.push_back(tempp);
				k_res = correct(i, j, 2);
				Err_log temppa(i + 1, j + 1, k_res, 's', 'g', pthread_self());
				err_log.push_back(temppa);
				cancel[2] = 0;
				canthread = d->thr_no;
				pthread_mutex_unlock(&mutex1);
				//output_f = 1;
			}
			else
				check[s[i][j] - 1] = true;
		}
	}

	pthread_mutex_lock(&mutex1);
	output_final[d->thr_no + 2] = output_f;
	pthread_mutex_unlock(&mutex1);
}
void handle_threads(int row, int col, int t_typ, bool x)
{
	if (x)
	{
		if (t_typ == 0) //row
		{
			if (th_id1 != 0)
			{
				Thrd_log tempp(pthread_self(), th_id1);
				thrd_log.push_back(tempp);
				pthread_cancel(th_id1);
				
			}
			if (th_id3[give_pos_thd(row, col)] != 0)
			{
				pthread_cancel(th_id3[give_pos_thd(row, col)]);
				Thrd_log temppa(pthread_self(), th_id3[give_pos_thd(row, col)]);
				thrd_log.push_back(temppa);
			}
		}
		else if (t_typ == 1) //col
		{
			if (th_id2 != 0)
			{
				Thrd_log tempp(pthread_self(), th_id2);
				thrd_log.push_back(tempp);
				pthread_cancel(th_id2);
				
			}
			if (th_id3[give_pos_thd(row, col)] != 0)
			{
				pthread_cancel(th_id3[give_pos_thd(row, col)]);
				Thrd_log temppa(pthread_self(), th_id3[give_pos_thd(row, col)]);
				thrd_log.push_back(temppa);
			}
		}
		else if (t_typ == 2) //grid
		{
			pthread_cancel(th_id2);
			Thrd_log temppa(pthread_self(), th_id2);
			thrd_log.push_back(temppa);
			pthread_cancel(th_id1);
			Thrd_log tempp(pthread_self(), th_id1);
			thrd_log.push_back(tempp);
		}
	}
	else
	{
		if (t_typ == 0) //row
		{
			pthread_create(&th_id1, NULL, colthread, NULL); //create thread to check columns
			if (th_id3[give_pos_thd(row, col)] != 0)
			{
				pthread_create(&th_id3[give_pos_thd(row, col)], NULL, gridthread, (void *)&d[give_pos_thd(row, col)]);
			}
		}
		else if (t_typ == 1) //col
		{
			pthread_create(&th_id2, NULL, rowthread, NULL); //create thread to check rows
			if (th_id3[give_pos_thd(row, col)] != 0)
			{
				pthread_create(&th_id3[give_pos_thd(row, col)], NULL, gridthread, (void *)&d[give_pos_thd(row, col)]);
			}
		}
		else if (t_typ == 2) //grid
		{
			pthread_create(&th_id1, NULL, colthread, NULL); //create thread to check columns
			pthread_create(&th_id2, NULL, rowthread, NULL); //create thread to check rows
		}
	}
}

int main()
{
	fstream o;						   //to read file
	pthread_mutex_init(&mutex1, NULL); //initialize mutex variable

	o.open("file.txt");
	int value = 1;
	for (int i = 0; i < 9; i++)
	{ //read file
		for (int j = 0; j < 9; j++)
		{
			o >> s[i][j]; //initialize main matrix with file values
		}
	}
	cout << endl
		 << "********** Input ***********" << endl;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << s[i][j] << " ";
		}
		cout << endl;
	}
	cout << "********** ----- ***********" << endl;
	pthread_create(&th_id1, NULL, colthread, NULL); //create thread to check columns
	pthread_create(&th_id2, NULL, rowthread, NULL); //create thread to check rows
	int thr_no = 0;
	for (int i = 0; i < 9; i += 3)
	{ //creating 9 different threads for 9 different grids
	cout << "wow: " << endl;
		for (int j = 0; j < 9; j += 3)
		{
			d[thr_no].row = i; //set starting row
			d[thr_no].col = j;
			d[thr_no].thr_no = thr_no;											   //set starting col
			pthread_create(&th_id3[thr_no], NULL, gridthread, (void *)&d[thr_no]); //create thread with arguments
			pthread_join(th_id3[thr_no], NULL);
			++thr_no;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		//pthread_join(th_id3[i], NULL);
	}
	pthread_join(th_id1, NULL);
	pthread_join(th_id2, NULL); //join thread
	int check = 0;
	cout << "check point 1" << endl;
	while(cancel[0] == 0 /*column thread found error*/
		|| cancel[1] == 0/*row thread found error*/
		)
	{	//recreating thread 
		cout << "im in" << endl;
		if(check == 15){
			cout << "it take too much time " << endl;
			cout << "near to complete";
			break;
		}
		pthread_mutex_unlock(&mutex1);
		pthread_create(&th_id1, NULL, colthread, NULL); //recreate thread to check columns
		pthread_create(&th_id2, NULL, rowthread, NULL); //recreate thread to check rows
	//	pthread_create(&th_id2, NULL, rowthread, NULL); //recreate thread to check rows
		pthread_join(th_id1, NULL);
		pthread_join(th_id2, NULL); //join thread
		check++;
	} 

	for (int i = 0; i < err_log.size(); ++i)
	{
		err_log[i].print_all();
	}
	cout << "Total no of errors : " << err_log.size() / 2 << endl;

	for (int i = 0; i < thrd_log.size(); ++i)
	{
		thrd_log[i].print_all();
	}
	cout << "Validation results of threads : " << endl;
	int kount = 1;
	for (int i = 0; i < 11; ++i)
	{
		if (i == 0)
		{
			cout << "Column Thread Result: " << output_final[i] << endl;
		}
		else if (i == 1)
		{
			cout << "Row Thread Result: " << output_final[i] << endl;
		}
		else
		{
			cout << "Grid Thread " << kount << " Reuslt: " << output_final[i] << endl;
			kount++;
		}
	}
	cout << endl
		 << "********** Output ***********" << endl;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << s[i][j] << " ";
			//w << s[i][j] << " "; 
		}
		cout << endl;
	}
	cout << "********** ----- ***********" << endl;
	return 0;
}
