#include <iostream>
#include <logs/logs.h>
#include "input.h"
#include "atom.h"

input::input() {}

input::~input() {}

void input::readPhaseSpace(atom *_atom, comm::BccDomain *p_domain) {
    particledata *sendbuf;
    _phaseSpaceFile = "dump.atom";
    kiwi::logs::e("phase-space", "Opening phase space file {}.\n", _phaseSpaceFile);
    _phaseSpaceFileStream.open(_phaseSpaceFile.c_str());
    if (!_phaseSpaceFileStream.is_open()) {
        kiwi::logs::e("phase-space", "Could not open phaseSpaceFile {}.\n ", _phaseSpaceFile);
    }
    kiwi::logs::e("phase-space", "Reading phase space file {}.\n", _phaseSpaceFile);
    double x, y, z, vx, vy, vz;
    unsigned long id;
    int type;
    x = y = z = vx = vy = vz = 0.;
    for (unsigned long i = 0; i < 432; i++) {
        _phaseSpaceFileStream >> id >> x >> y >> z >> vx >> vy >> vz;
        _atom->addAtom(p_domain, id, x, y, z, vx, vy, vz);
    }
    _phaseSpaceFileStream.close();
}
