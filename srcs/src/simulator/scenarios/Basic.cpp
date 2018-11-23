#include <iostream>
#include <ctime>
#include <omp.h>
#include "../mMath.h"
#include "Basic.h"

using namespace std;

Basic::Basic(int agent_n, int obs_n)
{
	agent_num = agent_n;
	obstacle_num = obs_n;

	initEvaluation();
	initWalls();

	Reset(-1);
}

void Basic::initWalls()
{
	wall_num = 0;
}

Basic::~Basic()
{
	for(vector< Agent* >::iterator it = _agents.begin() ; it != _agents.end(); it++)
		delete (*it);
	_agents.clear();

	for(vector< Obstacle* >::iterator it = _obstacles.begin() ; it != _obstacles.end(); it++)
		delete (*it);
	_obstacles.clear();
}

void Basic::initEvaluation()
{
	srand((unsigned int)time(0));

	for(int k=0; k<eval_set_num; k++)
	{
		vector<double> cur_obs_p;
		vector<double> cur_obs_r;
		for(int i=0; i<obstacle_num; i++)
		{
			double obs_pos[2];
			obs_pos[0] = -15.0 + rand()%30;
			obs_pos[1] = -15.0 + rand()%30;

			double obs_r[2];
			obs_r[0] = (10 + rand()%30)/20.0;
			obs_r[1] = (10 + rand()%30)/20.0;

			double tmp;
			if(obs_r[0] < obs_r[1]){
				tmp = obs_r[0];
				obs_r[0] = obs_r[1];
				obs_r[1] = tmp;
			}

			cur_obs_p.push_back(obs_pos[0]);
			cur_obs_p.push_back(obs_pos[1]);
			cur_obs_r.push_back(obs_r[0]);
			cur_obs_r.push_back(obs_r[1]);
		}
		eval_obs_p.push_back(cur_obs_p);
		eval_obs_r.push_back(cur_obs_r);

		vector<double> cur_agent_p;
		vector<double> cur_agent_r;
		vector<double> cur_agent_d;
		for(int i=0; i<agent_num; i++)
		{
			double agent_r[2];
			agent_r[0] = (10 + rand()%20)/20.0;
			agent_r[1] = (10 + rand()%20)/20.0;

			double tmp;
			if(agent_r[0] < agent_r[1]){
				tmp = agent_r[0];
				agent_r[0] = agent_r[1];
				agent_r[1] = tmp;
			}

			agent_r[1] = agent_r[0];

			bool col = false;
			double agent_pos[2];
			while(true)
			{
				agent_pos[0] = -15.0 + rand()%30;
				agent_pos[1] = -20.0 + rand()%40;

				col = false;
				int start_idx = 0;
				for(int j=start_idx; j<i; j++)
				{
					double boundary = cur_agent_r.at(j*2) + agent_r[0] + 0.1;
					double tmp_p[2];
					tmp_p[0] = cur_agent_p.at(j*2);
					tmp_p[1] = cur_agent_p.at(j*2+1);
					if(Dist(agent_pos, tmp_p) < boundary)
					{
						col = true;
						break;
					}
				}

				if(col == false)
				{
					for(int j=0; j<obstacle_num; j++)
					{
						double boundary = cur_obs_r.at(j*2) + agent_r[0];
						double tmp_p[2];
						tmp_p[0] = cur_obs_p.at(j*2);
						tmp_p[1] = cur_obs_p.at(j*2+1);
						if(Dist(agent_pos, tmp_p) < boundary)
						{
							col = true;
							break;
						}
					}
				}

				if(col == false)
					break;
			}

			bool d_col = false;
			double agent_d[2];
			while(true){
				d_col = false;
				agent_d[0] = -15 + rand()%30;
				agent_d[1] = -15 + rand()%30;

				for(int j=0; j<obstacle_num; j++)
				{
					double boundary = cur_obs_r.at(j*2) + 0.5;
					double tmp_p[2];
					tmp_p[0] = cur_obs_p.at(j*2);
					tmp_p[1] = cur_obs_p.at(j*2+1);
					if(Dist(agent_d, tmp_p) < boundary)
					{
						d_col = true;
						break;
					}
				}
				if(!d_col)
					break;
			}

			cur_agent_p.push_back(agent_pos[0]);
			cur_agent_p.push_back(agent_pos[1]);
			cur_agent_r.push_back(agent_r[0]);
			cur_agent_r.push_back(agent_r[1]);
			cur_agent_d.push_back(agent_d[0]);
			cur_agent_d.push_back(agent_d[0]);
		}
		eval_agent_p.push_back(cur_agent_p);
		eval_agent_r.push_back(cur_agent_r);
		eval_agent_d.push_back(cur_agent_d);
	}
}

void Basic::Reset(int idx)
{
	if(idx == -1)
		ResetEnv();
	else
		ResetEval(idx);
}

void Basic::ResetEval(int idx)
{
	for(vector< Agent* >::iterator it = _agents.begin() ; it != _agents.end(); it++)
		delete (*it);
	_agents.clear();

	for(vector< Obstacle* >::iterator it = _obstacles.begin() ; it != _obstacles.end(); it++)
		delete (*it);
	_obstacles.clear();

	for(int i=0; i<agent_num; i++)
	{
		double agent_r[2];
		agent_r[0] = eval_agent_r.at(idx).at(i*2);
		agent_r[1] = eval_agent_r.at(idx).at(i*2+1);

		Agent* agent = new Agent(agent_r);
		agent->setId(i);
		agent->setP(eval_agent_p.at(idx).at(i*2), eval_agent_p.at(idx).at(i*2+1));
		agent->setPprev(eval_agent_p.at(idx).at(i*2), eval_agent_p.at(idx).at(i*2+1));
		agent->setD(eval_agent_d.at(idx).at(i*2), eval_agent_d.at(idx).at(i*2+1));
		agent->setQy(1.0, 0.0);
		agent->setQx(0.0, -1.0);
		agent->setFront(0.0);
		agent->setColor(0.9, 0.1, 0.1);
		addAgent(agent);
	}

	for(int i=0; i<obstacle_num; i++)
	{
		double obstacle_r[2];
		obstacle_r[0] = eval_obs_r.at(idx).at(i*2);
		obstacle_r[1] = eval_obs_r.at(idx).at(i*2+1);

		double obstacle_p[2];
		obstacle_p[0] = eval_obs_p.at(idx).at(i*2);
		obstacle_p[1] = eval_obs_p.at(idx).at(i*2+1);

		Obstacle* obstacle = new Obstacle(obstacle_r, obstacle_p);
		addObstacle(obstacle);
	}

	_cur_step = 0;
}

void Basic::ResetEnv()
{
	for(vector< Agent* >::iterator it = _agents.begin() ; it != _agents.end(); it++)
		delete (*it);
	_agents.clear();

	for(vector< Obstacle* >::iterator it = _obstacles.begin() ; it != _obstacles.end(); it++)
		delete (*it);
	_obstacles.clear();

	srand((unsigned int)time(0));

	#pragma omp parallel for
	for(int i=0; i<obstacle_num; i++)
	{
		double obs_pos[2];
		obs_pos[0] = -10.0 + rand()%20;
		obs_pos[1] = -10.0 + rand()%20;

		double obs_r[2];
		obs_r[0] = (10 + rand()%30)/20.0;
		obs_r[1] = (10 + rand()%30)/20.0;

		double tmp;
		if(obs_r[0] < obs_r[1]){
			tmp = obs_r[0];
			obs_r[0] = obs_r[1];
			obs_r[1] = tmp;
		}

		Obstacle* obs = new Obstacle(obs_r, obs_pos); // p q d
		addObstacle(obs);
	}

	for(int i=0; i<agent_num; i++)
	{
		double agent_r[2];
		agent_r[0] = (10 + rand()%20)/20.0;
		agent_r[1] = (10 + rand()%10)/20.0;

		double tmp;
		if(agent_r[0] < agent_r[1]){
			tmp = agent_r[0];
			agent_r[0] = agent_r[1];
			agent_r[1] = tmp;
		}

		Agent* agent = new Agent(agent_r);
		agent->setId(i);

		bool col = false;
		double agent_pos[2];
		while(true)
		{
			agent_pos[0] = -12.0 + rand()%24;
			agent_pos[1] = -12.0 + rand()%24;

			col = false;
			int start_idx = 0;
			for(int j=start_idx; j<i; j++)
			{
				double boundary = (getAgent(j)->getR())[0] + agent_r[0] + 0.1;
				if(Dist(agent_pos, getAgent(j)->getP()) < boundary)
				{
					col = true;
					break;
				}
			}

			if(col == false)
			{
				for(int j=0; j<obstacle_num; j++)
				{
					double boundary = getObstacle(j)->getR()[0] + agent_r[0];
					if(Dist(agent_pos, getObstacle(j)->getP()) < boundary)
					{
						col = true;
						break;
					}
				}
			}

			if(col == false)
				break;
		}

		agent->setP(agent_pos[0], agent_pos[1]);
		agent->setPprev(agent_pos[0], agent_pos[1]);

		bool d_col = false;
		double d_pos[2];
		while(true){
			d_col = false;
			d_pos[0] = -10 + rand()%20;
			d_pos[1] = -10 + rand()%20;

			for(int j=0; j<obstacle_num; j++)
			{
				double boundary = getObstacle(j)->getR()[0] + 0.5;
				if(Dist(d_pos, getObstacle(j)->getP()) < boundary)
				{
					d_col = true;
					break;
				}
			}
			if(!d_col)
				break;
		}

		agent->setD( d_pos[0], d_pos[1]);
		agent->setQy(1.0, 0.0);
		agent->setQx(0.0, -1.0);
		agent->setFront(0.0);
		agent->setColor(0.9, 0.1, 0.1);
		if(i==0)
			agent->setColor(0.1, 0.9, 0.1);

		addAgent(agent);
	}
	_cur_step = 0;
}




