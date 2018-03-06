//
// Created by genshen on 2018-3-5.
//

#ifndef CRYSTAL_MD_HARDWARE_ACCELERATE_H
#define CRYSTAL_MD_HARDWARE_ACCELERATE_H

#include "pre_config.h"

#ifdef ARCH_SUNWAY

#include "arch/sunway/athread_accelerate.h" // sunway athread

#endif

// check whether it has accelerate hardware to be used, for example GPU, MIC(Xeon Phi), or sunway slave cores.
inline bool isAccelerateSupport() {
#ifdef ARCH_SUNWAY
    return true; // sunway.
#endif // todo other hardware.
    return false;
#endif
}

// initial for hardware accelerate.
inline void accelerateInit(int &lolocalx, int &lolocaly, int &lolocalz,
                           int &nlocalx, int &nlocaly, int &nlocalz,
                           int &loghostx, int &loghosty, int &loghostz,
                           int &nghostx, int &nghosty, int &nghostz) {
#ifdef ARCH_SUNWAY
    athreadAccelerateInit(lolocalx, lolocaly, lolocalz, nlocalx, nlocaly, nlocalz,
                          loghostx, loghosty, loghostz, nghostx, nghosty, nghostz);
#endif
}

// it runs after atom and boxes creation, but before simulation running.
inline void beforeAccelerateRun(eam *_pot) {
#ifdef ARCH_SUNWAY
    initSpline(_pot->rho->spline, _pot->f->spline, _pot->phi->spline);
#endif
}

// accelerate for calculating rho in computing eam potential.
inline void accelerateEamRhoCalc(int *rho_n, double *x, double *rho, double *cutoffRadius,
                                 double *rhoInvDx, double *rhoSplineValues) {
#ifdef ARCH_SUNWAY
    athreadAccelerateEamRhoCalc(rho_n, x, rho, cutoffRadius, rhoInvDx, rhoSplineValues);
#endif
}

// accelerate for calculating df in computing eam potential.
inline void accelerateEamDfCalc(int *df_n, double *rho, double *df, double *cutoffRadius,
                                double *dfSplineInvDx, double *dfSplineValues) {
#ifdef ARCH_SUNWAY
    athreadAccelerateEamDfCalc(df_n, rho, df, cutoffRadius, dfSplineInvDx, dfSplineValues);
#endif
}

// accelerate for calculating force in computing eam potential.
inline void accelerateEamForceCalc(int *phi_n, double *x, double *f, double *df,
                                   double *cutoffRadius, double *phiSplineInvDx,
                                   double *phiSplineValues, double *rhoSplineValues) {
#ifdef ARCH_SUNWAY
    athreadAccelerateEamForceCalc(phi_n, x, f, df, cutoffRadius, phiSplineInvDx, phiSplineValues, rhoSplineValues);
#endif
}

#endif //CRYSTAL_MD_HARDWARE_ACCELERATE_H