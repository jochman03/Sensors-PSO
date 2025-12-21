#ifndef PSO_H
#define PSO_H

#include <stdint.h>
#include <raylib.h>

// Structure for 2D point
typedef struct {
    double x;
    double y;
} m_point;



// Budget limit
extern double maxCost;

// Candidate sensor placement points
extern m_point* sensorPoints;
extern uint32_t sensorPointsCount;



// Sensor type parameters

// cost per sensor type
extern double* sensorCost;            
// radius per sensor type
extern double* sensorRange;           
// max count per sensor type
extern uint32_t* sensorAvailability;  
// number of types (including 0)
extern uint8_t sensorTypeCount;       

// Points that must be covered
extern m_point* points;
extern uint32_t pointsCount;
// required coverage level per point
extern uint8_t* pointsCoverage;



// PSO run settings
extern uint32_t maxParticles;
extern uint32_t maxIterations;
extern uint32_t simulationProgress;


// UI control flags
extern bool startSimulation;
extern bool simulationDone;



/* --- Results --- */

// Best sensor type chosen for each sensor point
extern uint32_t* psoSolution;

// Coverage achieved per point (size: pointsCount)
extern uint32_t* coverSolution;

// Used sensors per type (size: sensorTypeCount)
extern uint32_t* typeCountSolution;

extern uint32_t uncoveredSolution;
extern double totalCostSolution;
extern double psoValue;



/* --- PSO coefficients and penalties --- */
extern double c1, c2, w;

// missing coverage penalty
extern uint64_t coverPenalty;
// exceeding availability penalty
extern uint64_t quantityPenalty;
// exceeding budget penalty
extern uint64_t budgetPenalty;



// Allocate/init internal buffers and swarm
void psoStart(void);

// Run a chunk of iterations (call per frame)
void psoHandle(void);

// Evaluate solution vector (length: sensorPointsCount)
double objectiveFun(double* particlesPos);

#endif // PSO_H
