/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.

***************************************************************************************************/

#pragma once

#include "BoostLibWrapper.h"
#include "Individual.h"

namespace Kernel
{
    class IndividualHumanAirborne : public IndividualHuman
    {
    public:    
        DECLARE_QUERY_INTERFACE()

        virtual ~IndividualHumanAirborne(void) { }
        static   IndividualHumanAirborne *CreateHuman(INodeContext *context, suids::suid _suid, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);

        // Infections and Susceptibility
        virtual void  CreateSusceptibility(float=1.0, float=1.0);

    protected:
        IndividualHumanAirborne(suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);

        virtual Infection* createInfection(suids::suid _suid);
        virtual void ReportInfectionState();

    private:

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, IndividualHumanAirborne& human, const unsigned int  file_version );
#endif
    };
}
