#include "pso.h"
#include "menu.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define POW2(a) ((a) * (a))

double maxCost;
m_point* sensorPoints;
uint32_t sensorPointsCount;

double* sensorCost;
double* sensorRange;
uint32_t* sensorAvailability;

uint8_t sensorTypeCount;

m_point* points;
uint32_t pointsCount;
uint8_t* pointsCoverage;

uint32_t maxParticles;
uint32_t maxIterations;

uint32_t simulationProgress = 0;


uint32_t* psoSolution;
uint32_t* coverSolution;
uint32_t* typeCountSolution;
uint32_t uncoveredSolution = 0;
double psoValue;
double totalCostSolution;

bool *cov = NULL;

double* particlesPos;
double* particlesVel;
double* particlesBestPos;
double* particlesBestVal;
double* globalBestPos;
double globalBestVal = 1e10;

int32_t* roundedPos;
uint32_t* coverV;
uint32_t* typeCount;

bool startSimulation = 0;
bool simulationDone = 0;

double c1 = 1.5;
double c2 = 1.5;
double w = 0.7;
double vmax = 2.0f;

uint64_t coverPenalty = 1e3;
uint64_t quantityPenalty = 1e10;
uint64_t budgetPenalty = 2e3;

static void covMatrix(bool* cov){
    for(uint32_t s = 0; s < sensorPointsCount; s++){
        for(uint32_t p = 0; p < pointsCount; p++){
            for(uint32_t r = 0; r < sensorTypeCount; r++){
                uint32_t idx = s * (pointsCount * sensorTypeCount) + p * sensorTypeCount + r;
                cov[idx] = (POW2(sensorPoints[s].x - points[p].x) + POW2(sensorPoints[s].y - points[p].y) <= POW2(sensorRange[r]));
            }
        }
    }
}

double objectiveFun(double* particlesPos){
    for(uint32_t i = 0; i < sensorPointsCount; i++){
        roundedPos[i] = (int32_t)lround(particlesPos[i]);
        if(roundedPos[i] < 0){
            roundedPos[i] = 0;
        }
        else if(roundedPos[i] > sensorTypeCount - 1){
            roundedPos[i] = sensorTypeCount - 1;
        }
    }
    for(uint32_t p = 0; p < pointsCount; p++){
        coverV[p] = 0;
    }

    for(uint8_t r = 0; r < sensorTypeCount; r++){
        typeCount[r] = 0;
    }
    double totalCost = 0;
    for (uint32_t i = 0; i < sensorPointsCount; i++) {
        uint32_t r = (uint32_t)roundedPos[i];
        if(r == 0){
            continue;
        }
        typeCount[r]++;
        totalCost += sensorCost[r];
    }

    for (uint32_t p = 0; p < pointsCount; p++) {
        uint32_t covered = 0;
        for (uint32_t i = 0; i < sensorPointsCount; i++) {
            uint32_t r = (uint32_t)roundedPos[i];
            if(r == 0){
                continue;
            }
            uint32_t idx = i * (pointsCount * sensorTypeCount) + p * sensorTypeCount + r;
            covered += cov[idx];
        }
        coverV[p] = covered;
    }
    double coverCost = 0;
    for (uint32_t p = 0; p < pointsCount; p++) {
        uint32_t need = pointsCoverage[p];
        uint32_t have = coverV[p];
        if (have < need) coverCost += (double)(need - have) * coverPenalty;
    }

    double quantityCost = 0;
    for (uint32_t t = 0; t < sensorTypeCount; t++) {
        uint32_t need = typeCount[t];
        uint32_t have = sensorAvailability[t];
        if (need > have) quantityCost += (double)(need - have) * quantityPenalty;
    }

    double budgetCost = 0;
    if ((double)totalCost > (double)maxCost) {
        budgetCost = (double)(((double)totalCost - (double)maxCost) * (double)budgetPenalty);
    }

    return (double)((double)budgetCost + (double)quantityCost + (double)coverCost + (double)totalCost);

}

void psoStart(){
    simulationProgress = 0;
    simulationDone = 0;

    if(cov){
        free(cov);
        cov = NULL;
    }
    if(typeCount){
        free(typeCount);
        typeCount = NULL;
    }
    if(coverV){
        free(coverV);
        coverV = NULL;
    }
    if(roundedPos){
        free(roundedPos);
        roundedPos = NULL;
    }
    if(particlesPos){
        free(particlesPos);
        particlesPos = NULL;
    }
    if(particlesVel){
        free(particlesVel);
        particlesVel = NULL;
    }
    if(particlesBestPos){
        free(particlesBestPos);
        particlesBestPos = NULL;
    }
    if(particlesBestVal){
        free(particlesBestVal);
        particlesBestVal = NULL;
    }
    if(globalBestPos){
        free(globalBestPos);
        globalBestPos = NULL;
    }

    cov = malloc(pointsCount * sensorPointsCount * sensorTypeCount * sizeof(bool));
    if (!cov) {
        exit(0);
    }
    covMatrix(cov);

    typeCount = malloc(sensorTypeCount * sizeof(uint32_t));
    coverV = malloc(pointsCount * sizeof(uint32_t));
    roundedPos = malloc(sensorPointsCount * sizeof(int32_t));

    if(!typeCount || !coverV || !roundedPos){
        exit(0);
    }

    particlesPos = malloc(maxParticles * sensorPointsCount * sizeof(double));
    if(!particlesPos){
        exit(0);
    }
    particlesVel = malloc(maxParticles * sensorPointsCount * sizeof(double));
    if(!particlesVel){
        exit(0);
    }
    particlesBestPos = malloc(maxParticles * sensorPointsCount * sizeof(double));
    if(!particlesBestPos){
        exit(0);
    }
    particlesBestVal = malloc(maxParticles * sizeof(double));
    if(!particlesBestVal){
        exit(0);
    }
    globalBestPos = malloc(sensorPointsCount * sizeof(double));
    if(!globalBestPos){
        exit(0);
    }
    globalBestVal = 1e10;
    for(uint32_t p = 0; p < maxParticles; p++){
        for(uint32_t s = 0; s < sensorPointsCount; s++){
            double pos = ((double)rand() / (double)RAND_MAX) * sensorTypeCount;
            particlesPos[p * sensorPointsCount + s] = pos;
            particlesBestPos[p * sensorPointsCount + s] = pos;
            particlesVel[p * sensorPointsCount + s] = (double)rand() / (double)RAND_MAX;
        }
        double particleVal = objectiveFun(&particlesPos[p * sensorPointsCount]);
        particlesBestVal[p] = particleVal;
        if (particleVal < globalBestVal) {
            globalBestVal = particleVal;
            for (uint32_t s = 0; s < sensorPointsCount; s++) {
                globalBestPos[s] = particlesPos[p * sensorPointsCount + s];
            }
        }
    }
}

static void psoStop(void){
    free(cov);
    free(particlesPos);
    free(particlesVel);
    free(particlesBestPos);
    free(particlesBestVal);
    free(globalBestPos);
    free(roundedPos);
    free(coverV);
    free(typeCount);

    cov = NULL;
    particlesPos = NULL;
    particlesVel = NULL;
    particlesBestPos = NULL;
    particlesBestVal = NULL;
    globalBestPos = NULL;
    roundedPos = NULL;
    coverV = NULL;
    typeCount = NULL;
}

void psoHandle(){
    uint32_t done = 0;
    while (done < 50 && simulationProgress < maxIterations) {
        for(uint32_t p = 0; p < maxParticles; p++){
            for(uint32_t s = 0; s < sensorPointsCount; s++){
                uint32_t idx = p * sensorPointsCount + s;
                double r1 = (double)rand() / (double)RAND_MAX;
                double r2 = (double)rand() / (double)RAND_MAX;
                particlesVel[idx] = w * particlesVel[idx] 
                + c1 * r1 * (particlesBestPos[idx] - particlesPos[idx]) 
                + c2 * r2 * (globalBestPos[s] - particlesPos[idx]);
                double pos = particlesPos[idx] + particlesVel[idx];

                if (particlesVel[idx] >  vmax) {
                    particlesVel[idx] =  vmax;
                }
                if (particlesVel[idx] < -vmax) {
                    particlesVel[idx] = -vmax;
                }
                if (pos < 0.0f) {
                    pos = 0.0f;
                }
                if (pos > (double)(sensorTypeCount - 1)) {
                    pos = (double)(sensorTypeCount - 1);
                }
                particlesPos[idx] = pos;
            }

            double particleVal = objectiveFun(&particlesPos[p * sensorPointsCount]);
            if(particleVal < globalBestVal){
                globalBestVal = particleVal;
                for(uint32_t s = 0; s < sensorPointsCount; s++){
                    globalBestPos[s] = particlesPos[p * sensorPointsCount + s];
                }
            }
            if(particleVal < particlesBestVal[p]){
                particlesBestVal[p] = particleVal;
                for(uint32_t s = 0; s < sensorPointsCount; s++){
                    uint32_t idx = p * sensorPointsCount + s;
                    particlesBestPos[idx] = particlesPos[idx];
                }
            }
        }
        simulationProgress++;
        done++;
    }
    if(simulationProgress < maxIterations){
        return;
    }
    if(!psoSolution || !coverSolution || !typeCountSolution){
        exit(0);
    }
    objectiveFun(globalBestPos);
    totalCostSolution = 0;
    for(uint32_t s = 0; s < sensorPointsCount; s++){
        psoSolution[s] = (int32_t)lround(globalBestPos[s]);
        if(psoSolution[s] < 0){
            psoSolution[s] = 0;
        }
        else if(psoSolution[s] > sensorTypeCount - 1){
            psoSolution[s] = sensorTypeCount - 1;
        }
        totalCostSolution += sensorCost[psoSolution[s]];
    }
    for(uint32_t t = 0; t < sensorTypeCount; t++){
        typeCountSolution[t] = typeCount[t];
    }
    for(uint32_t p = 0; p < pointsCount; p++){
        coverSolution[p] = coverV[p];
    }

    psoValue = globalBestVal;
    uncoveredSolution = 0;
    for(uint32_t i = 0; i < pointsCount; i++){
        if(pointsCoverage[i] > coverV[i]){
            uncoveredSolution++;
        }
    }
    simulationDone = 1;
    startSimulation = 0;
    psoStop();
}

