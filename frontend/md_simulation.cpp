//
// Created by genshen on 2019-06-10.
//

#include <system_configuration.h>
#include <logs/logs.h>
#include <iostream>
#include <utils/mpi_domain.h>
#include "md_simulation.h"
#include "io/output_dump.h"
#include "io/output_copy.h"

MDSimulation::MDSimulation(ConfigValues *p_config_values)
        : simulation(), pConfigVal(p_config_values), cur_step_length(p_config_values->timeStepLength) {}

void MDSimulation::onSimulationStarted() {
    switch (pConfigVal->output.atomsDumpMode) {
        case OutputMode::DEBUG:
            out = new OutputDump(pConfigVal->output, *_p_domain);
            break;
        case OutputMode::COPY:
            out = new OutputCopy(pConfigVal->output, *_p_domain);
            break;
    }
}

void MDSimulation::onSimulationDone(const unsigned long step) {
    out->onAllOut(step);
    delete out;
}

void MDSimulation::beforeStep(const unsigned long step) {
    for (size_t i = 0; i < pConfigVal->vsl_size; i++) {
        if (step == pConfigVal->vsl_break_points[i]) {
            _newton_motion->setTimestepLength(pConfigVal->vsl_lengths[i]);
            cur_step_length = pConfigVal->vsl_lengths[i];
        }
    }

    kiwi::logs::s(MASTER_PROCESSOR, "simulation", "simulating steps: {}/{}\r",
                  step + 1, pConfigVal->timeSteps);

    if (step == pConfigVal->collisionStep && !pConfigVal->output.originDumpPath.empty()) {
        // output atoms in system before collision.
        // step not plus 1 because it just start time step (step+1).
        out->beforeCollision(step, _atom->getAtomList(), _atom->getInterList());
    }
}

void MDSimulation::postStep(const unsigned long step) {
    phy_time += cur_step_length; // the physical time when this iteration is finished.
    kiwi::logs::i(MASTER_PROCESSOR, "simulation", "simulated physical time: {} ps.\n", phy_time);

    // output atoms information if it is dumping step
    if ((step + 1) % pConfigVal->output.atomsDumpInterval == 0) {
        out->onOutputStep(step + 1, _atom->getAtomList(), _atom->getInterList());
    }

    // output thermodynamics information if it it the step
    if (pConfigVal->output.thermo_interval && (step + 1) % pConfigVal->output.thermo_interval == 0) {
        const double e = configuration::kineticEnergy(_atom->getAtomList(), _atom->getInterList(),
                                                      configuration::ReturnMod::Root, MASTER_PROCESSOR);
        const _type_atom_count n = 2 * pConfigVal->phaseSpace[0] *
                                   pConfigVal->phaseSpace[1] * pConfigVal->phaseSpace[2];
        const double T = configuration::temperature(e, n);
        kiwi::logs::i(MASTER_PROCESSOR, "energy", "kinetic energy = {}, T = {}.\n", e, T);
    }

#ifdef MD_DEV_MODE
    {
        unsigned long count[2] = {0, 0};
        count[0] = _atom->realAtoms();
        count[1] = _atom->getInterList()->nlocalinter;
        unsigned long global_count[2] = {0, 0};
        unsigned long &real_atoms = global_count[0], &inter_atoms = global_count[1];
        MPI_Reduce(count, global_count, 2, MPI_UNSIGNED_LONG, MPI_SUM, MASTER_PROCESSOR, MPI_COMM_WORLD);

        kiwi::logs::d("count", "real:{}--inter: {}\n", count[0], count[1]);
        kiwi::logs::d(MASTER_PROCESSOR, "count", "global_real:{}--global_inter: {}\n", real_atoms, inter_atoms);
    }
#endif
}

void MDSimulation::onForceSolved(const unsigned long step) {
#ifdef MD_DEV_MODE
    forceChecking();
#endif
}

void MDSimulation::print_force(const std::string filename, int step) {
    std::ofstream outfile;
    outfile.open(filename);

    _atom->atom_list->foreachSubBoxAtom(
            [&outfile](AtomElement &_atom_ref) {
                outfile << _atom_ref.id << "\t"
                        << _atom_ref.x[0] << "\t" << _atom_ref.x[1] << "\t" << _atom_ref.x[2] << "\t"
                        << _atom_ref.v[0] << "\t" << _atom_ref.v[1] << "\t" << _atom_ref.v[2] << "\t"
                        << _atom_ref.f[0] << "\t" << _atom_ref.f[1] << "\t" << _atom_ref.f[2] << std::endl;
            }
    );
    outfile << "inter atoms" << std::endl;
    for (_type_inter_list::iterator inter_it = _atom->inter_atom_list->inter_list.begin();
         inter_it != _atom->inter_atom_list->inter_list.end(); inter_it++) {
        AtomElement &_atom_ref = *inter_it;
        outfile << std::endl << _atom_ref.id << "\t"
                << _atom_ref.x[0] << "\t" << _atom_ref.x[1] << "\t" << _atom_ref.x[2] << "\t"
                << _atom_ref.v[0] << "\t" << _atom_ref.v[1] << "\t" << _atom_ref.v[2] << "\t"
                << _atom_ref.f[0] << "\t" << _atom_ref.f[1] << "\t" << _atom_ref.f[2] << std::endl;
    }
    outfile.close();
}


#ifdef MD_DEV_MODE

void MDSimulation::forceChecking() {
    auto forces = configuration::systemForce(_atom->getAtomList(), _atom->getInterList());
    double fx[3] = {forces[0], forces[1], forces[2]};
    double fx_2[3] = {0.0, 0.0, 0.0};
    MPI_Allreduce(fx, fx_2, 3, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    kiwi::logs::d("force2", "({}, {}, {})\n\n", forces[0], forces[1], forces[2]);
    kiwi::logs::d(MASTER_PROCESSOR, "sum force:", "({}, {}, {})\n\n", fx_2[0], fx_2[1], fx_2[2]);
    if (std::abs(fx_2[0]) > 0.00001) {
        char filename[20];
        sprintf(filename, "force_%d.txt", kiwi::mpiUtils::global_process.own_rank);
        print_force(filename, _simulation_time_step);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Abort(MPI_COMM_WORLD, 0);
    }
}

#endif //MD_DEV_MODE
