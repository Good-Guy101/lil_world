#include<stdio.h>
#include<stdlib.h>

#include "raylib.h"
#include "raymath.h"

typedef struct Observation{
	Vector2 position;
	Vector2 velocity;

	int num_peers;
	Vector2* peers_rel_p;
	Vector2* peers_rel_v;
} Observation;

typedef enum Action{
	NO_ACTION,
	LEFT,
	RIGHT,
	DOWN,
	UP
}

typedef struct Agent {
	int id;

	Vector2 pos;
	Vector2 vel;

	void* policy;
	enum Action (*get_action)(void*, Observation);

	float last_reward;
	Observation last_obs;
	enum Action last_action;

	Observation* obs;
	enum Actions* actions;
	float* rewards;
} Agent;

typedef struct Environment{
	Agent* agents;
	int num_agents;
	float time;
	float time_step;

	float world_width;
	float world_height;

	int observation_size;
	Observation (*get_obs)(Environment*,int);
	
	int action_size;
	void (*apply_action)(Environment*, int, enum Action); 

	float cum_reward;
	float (*reward_func)(Environment*);
} Environment;

enum Action dummy_get_action(void* policy, Observation obs)
{
	return GetRandomValue(0, 4);
}

void dummy_apply_action(Environment* env, int cur_agent){

}	


Environment* new_env(
	int num_agents, 
	float width, float height, 
	int action_size, 
	int observation_size, 
	float time_step, 
	float (*reward_func)(Environment*))
{
	Environment* env = (Environment*)malloc(sizeof(Environment));

	if(env == NULL){
		printf("ERROR: Could not allocate memory for new env");
		return NULL;
	}

	env->agents = (Agent*)malloc(sizeof(Agent) * num_agents);

	for(int i = 0; i < num_agents; i++){
		env->agents[i].id = i;
		env->agents[i].vel = (Vector2){0.0f};
		env->agents[i].pos = (Vector2){0.0f};

		env->agents[i].policy = NULL;
		env->agents[i].get_action = dummy_get_action;
	}

	env->world_width = width;
	env->world_height = height;

	env->action_size = action_size;
	env->observation_size = observation_size;

	env->time_step = time_step;
	env->time = 0.0f;

	env->cum_reward = 0.0f;

	env->reward_func = reward_func;

	return env;
}

Observation* get_obs(Environment* env, int cur_agent)
{
	Observation* obs = (Observation*)malloc(sizeof(Observation));

	obs->position = env->agents[cur_agent].pos;
	obs->velocity = env->agents[cur_agent].vel;

	obs->num_peers = env->num_agents - 1;
	obs->peers_rel_p = (Vector2*)malloc(sizeof(Vector2) * obs->num_peers);
	obs->peers_rel_v = (Vector2*)malloc(sizeof(Vector2) * obs->num_peers);

	int agent = 0;

	for(int i = 0; i < env->num_agents; i++){
		if(i == cur_agent)
			continue;

		obs->peers_rel_p[agent] = env->agents[i].pos;
		obs->peers_rel_v[agent] = env->agents[i].vel;
		agent++;
	}

	return obs;
}

float step_env(Environment* env)
{
	float total_reward = 0.0f;

	// Get agent observations 
	for(int i = 0; i < env->num_agents; i++){
		env->agents[i].last_obs = get_obs(env, i);
		env->agents[i].last_action = env->agents[i].get_action(
			env->agents[i].policy, 
			env->agents[i].last_obs
		);
	}

	// Apply actions
	for(int i = 0; i < env->num_agents; i++){
		apply_action(env, i);
	}

	// Calculate rewards
	for(int i = 0; i < env->num_agents; i++){
		
	}

	env->time += env->time_step;

	if(env->)
}

void run_env(Environment* env, int max_steps)
{
	float rewards[max_steps];

	for(int i = 0; i < max_steps; i++){
		rewards[i] = step_env(env);
	}
}