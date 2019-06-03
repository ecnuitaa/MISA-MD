//
// Created by genshen on 2019-04-25.
//

#ifndef CRYSTALMD_SYSTEM_CONFIGURATION_H
#define CRYSTALMD_SYSTEM_CONFIGURATION_H


#include <array>
#include <utils/data_def.h>

#include "types/pre_define.h"
#include "atom/atom_list.h"
#include "atom/inter_atom_list.h"

/**
 * system configuration:
 * Api of statistical values of simulation system,
 * like temperature, system force, momentum, kinetic energy and potential energy.
 */
namespace configuration {
    /**
     * return mode in global communication.
     */
    enum ReturnMod {
        Local, // local mode: each process in communication domain return its local data (no reduction operation).
        Root, // root mode: MPI_Reduce should be called to reduce data to root processes in communication domain.
        All, // all mode: MPI_ALlReduce should be called to reduce data to all processes in communication domain.
    };

    /**
     * compute the total force in each dimension of system.
     * @param atom_list pointer to lattice atoms list.
     * @param inter_atom_list pointer to inter atom list.
     * @return the total system force of in each dimension.
     */
    std::array<_type_atom_force, DIMENSION> systemForce(AtomList *atom_list, InterAtomList *inter_atom_list);

    /**
     * get kinetic energy of system.
     * @param atom_list pointer to lattice atoms list.
     * @param inter_atom_list pointer to inter atom list.
     * @param mode return mode of energy.
     * @param root the MPI root processes to receive the energy value in root return mode.
     * @return the kinetic energy of system.
     */
    double kineticEnergy(AtomList *atom_list, InterAtomList *inter_atom_list, ReturnMod mode, const kiwi::RID root);

    /**
     * kinetic energy lattice atom in system.
     * @param atom_list pointer to lattice atoms list.
     * @param mode return mode of energy.
     * @param root the MPI root processes to receive the energy value in root return mode.
     * @return the kinetic energy of all lattice atom in system.
     */
    double kineticEnergy(AtomList *atom_list, ReturnMod mode, const kiwi::RID root);

    /**
     * kinetic energy inter atom in system.
     * @param inter_atom_list pointer to inter atom list.
     * @param mode return mode of energy.
     * @param root the MPI root processes to receive the energy value in root return mode.
     * @return the kinetic energy of all inter atom in system.
     */
    double kineticEnergy(InterAtomList *inter_atom_list, ReturnMod mode, const kiwi::RID root);

    /**
     * convert kinetic energy to temperature.
     * @param ke kinetic energy of system.
     * @param n the count of particles in system.
     * @return temperature of system.
     */
    double temperature(const double ke, const _type_lattice_size n);
};


#endif //CRYSTALMD_SYSTEM_CONFIGURATION_H