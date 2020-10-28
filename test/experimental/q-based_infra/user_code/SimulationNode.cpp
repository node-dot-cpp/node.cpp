// HttpServerSample.cpp : sample of user-defined code

#ifndef NODECPP_USE_Q_BASED_INFRA
#include <infrastructure.h>
#else
#include <q_based_infrastructure.h>
#endif // NODECPP_USE_Q_BASED_INFRA
#include "SimulationNode.h"

static NodeRegistrator<Runnable<SampleSimulationNode>> noname( "SampleSimulationNode" );
