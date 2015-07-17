/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.

***************************************************************************************************/

#pragma once

#include <utility>
#include "InfectionVector.h"

#include "BoostLibWrapper.h"
#include "Common.h"
#include "MalariaContexts.h"
#include "MalariaEnums.h"
#include "IMalariaAntibody.h"
#include "SusceptibilityMalaria.h"

#include "Common.h"

namespace Kernel
{
    class IInfectionMalaria : public ISupports
    {
    public:
        virtual int64_t get_MaleGametocytes(int stage) = 0;
        virtual void    reset_MaleGametocytes(int stage) = 0;

        virtual int64_t get_FemaleGametocytes(int stage) = 0;
        virtual void    reset_FemaleGametocytes(int stage) = 0;
    };

    class InfectionMalariaConfig : public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(InfectionMalariaConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        InfectionMalariaConfig() {};
        bool Configure( const Configuration* config );

    protected:
        static ParasiteSwitchType::Enum parasite_switch_type;
        static MalariaStrains::Enum     malaria_strains;

        static double antibody_IRBC_killrate;
        static double MSP1_merozoite_kill;
        static double gametocyte_stage_survival;
        static double base_gametocyte_sexratio;
        static double base_gametocyte_production;
        static double antigen_switch_rate;
        static double merozoites_per_hepatocyte;
        static double merozoites_per_schizont;
        static double non_specific_antigenicity;
        static double RBC_destruction_multiplier;
        static int    n_asexual_cycles_wo_gametocytes;
    };

    class InfectionMalaria : public IInfectionMalaria, public InfectionVector, protected InfectionMalariaConfig
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        static InfectionMalaria *CreateInfection( IIndividualHumanContext *context, suids::suid suid, int initial_hepatocytes=1 );
        virtual ~InfectionMalaria();

        // TODO - becomes part of initialize?
        virtual void SetParameters(Kernel::StrainIdentity* _infstrain=NULL, int incubation_period_override = -1 );
        virtual void InitInfectionImmunology(Susceptibility* _immunity);

        virtual void Update(float, Kernel::Susceptibility * = NULL);

        // TODO: intrahost_report needs to be reimplemented as a custom reporter

        // DrugResistance Interface
        int getDrugResistanceFlag(void);

        // Sums up the current parasite counts for the infection and determines if the infection is cleared or if death occurs this time step
        void malariaCheckInfectionStatus(float = 0.0f, IMalariaSusceptibility * = NULL);

        // Calculates the IRBC killing from drugs and immune action
        void malariaImmunityIRBCKill(float = 0, IMalariaSusceptibility * = NULL);

        // Calculates stimulation of immune system by malaria infection
        void malariaImmuneStimulation(float = 0, IMalariaSusceptibility * = NULL);

        // Calculates immature gametocyte killing from drugs and immune action
        void malariaImmunityGametocyteKill(float = 0, IMalariaSusceptibility * = NULL);

        // Calculates the antigenic switching when an asexual cycle completes and creates next generation of IRBC's
        void malariaIRBCAntigenSwitch(double = 1.0);

        // Moves all falciparum gametocytes forward a development stage when an asexual cycle completes, and creates the stage 0 immature gametocytes
        void malariaCycleGametocytes(double = 1.0);

        // Process all infected hepatocytes
        void malariaProcessHepatocytes(float = 0, IMalariaSusceptibility * = NULL);

        virtual int64_t get_MaleGametocytes(int stage);
        virtual void    reset_MaleGametocytes(int stage);

        virtual int64_t get_FemaleGametocytes(int stage);
        virtual void    reset_FemaleGametocytes(int stage);

        virtual void SetContextTo(IIndividualHumanContext* context);

    protected:
        const SimulationConfig *params();
        void processEndOfAsexualCycle( IMalariaSusceptibility* immunity );

    private:
        // duration, incubation period, and infectious period are reused with different meanings from Infection, and Infection_Vector
        double m_IRBCtimer;
        int32_t m_hepatocytes;
        AsexualCycleStatus::Enum m_asexual_phase;
        int32_t m_asexual_cycle_count;

        int32_t m_MSPtype;        // allow variation in MSP from clone to clone
        int32_t m_nonspectype;    // what is the set of minor_epitope_types
        int32_t m_minor_epitope_type[CLONAL_PfEMP1_VARIANTS]; // TODO: these could be vectors that get cleared after setting up antibodies in InitInfectionImmunology...
        int32_t m_IRBCtype[CLONAL_PfEMP1_VARIANTS];

        // Refactored antigen-antibody interaction.  Keep pointers to antibody objects instead of variant indices.
        IMalariaAntibody* m_MSP_antibody; // TODO: would be nice to protect this from changing (const pointer) but it is only set in InitInfectionImmunology        
        std::vector< pfemp1_antibody_t > m_PfEMP1_antibodies;

        std::vector<int64_t> m_IRBC_count;
        int64_t m_malegametocytes[GametocyteStages::Count];
        int64_t m_femalegametocytes[GametocyteStages::Count];

        // govern distribution of next merozoites
        double m_gametorate;
        double m_gametosexratio;

        double m_measured_duration;
        bool m_start_measuring;
        double m_temp_duration;
        int64_t m_max_parasites;
        double m_inv_microliters_blood;   // tracks blood volume based on age

        InfectionMalaria();
        InfectionMalaria(IIndividualHumanContext *context);
        void Initialize(suids::suid suid, int initial_hepatocytes);
        std::string ValidateGametocyteCounts() const;
        std::string ValidateIRBCCounts() const;

        // drug resistance flag
        int drugResistanceFlag;

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, InfectionMalaria& inf, const unsigned int file_version );
#endif

#if USE_JSON_SERIALIZATION || USE_JSON_MPI
    public:
     // IJsonSerializable Interfaces
     virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const;
     virtual void JDeserialize( IJsonObjectAdapter* root, JSerializer* helper );
#endif
    };
}
