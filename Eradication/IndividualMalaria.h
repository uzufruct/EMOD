/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.

***************************************************************************************************/

#pragma once

#include "BoostLibWrapper.h"

#include "IndividualVector.h"
#include "IMalariaAntibody.h"
#include "Malaria.h"
#include "MalariaContexts.h"

#include "Common.h"

namespace Kernel
{
    struct IIndividualHumanEventContext;
    struct IIndividualHumanInterventionsContext;
    struct IIndividualHumanEventContext;

    // TBD: Make separate IndividualHumanMalariaConfig class!?!?!?!
    class IndividualHumanMalaria : public IndividualHumanVector, public IMalariaHumanContext, public IMalariaHumanInfectable
    {
        friend class SimulationMalaria;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(IndividualHumanMalaria);

    public:
        static IndividualHumanMalaria *CreateHuman(INodeContext *context, suids::suid _suid, double monte_carlo_weight = 1.0f, double initial_age = 0.0f, int gender = 0, double initial_poverty = 0.5f);
        virtual ~IndividualHumanMalaria();

        // IMalariaHumanContext methods
        virtual void  PerformMalariaTest(int test_type);
        virtual void  CountPositiveSlideFields(RANDOMBASE* rng, int nfields, float uL_per_field, int& positive_asexual_fields, int& positive_gametocyte_fields) const;
        virtual bool  CheckForParasitesWithTest(int test_type) const;
        virtual float CheckParasiteCountWithTest(int test_type) const;
        virtual float CheckGametocyteCountWithTest(int test_type) const; // the value with sensitivity and variability of a blood test
        virtual float GetGametocyteDensity() const;                      // the exact value in the model
        virtual bool  HasFever() const;
        virtual void  AddClinicalSymptom(ClinicalSymptomsEnum::Enum symptom);
        virtual bool  HasClinicalSymptom(ClinicalSymptomsEnum::Enum symptom) const;
        virtual IMalariaSusceptibility* GetMalariaSusceptibilityContext() const; // TBD: Get rid of this and use QueryInterface instead
        virtual std::vector< std::pair<int,int> > GetInfectingStrainIds() const;

        // IMalariaHumanInfectable methods
        virtual bool ChallengeWithBites( int n_infectious_bites );
        virtual bool ChallengeWithSporozoites( int n_sporozoites );

        virtual void CreateSusceptibility(float = 1.0f, float = 1.0f); // TODO: why isn't this protected (EAW)
        virtual void ClearNewInfectionState();
        virtual void setupMaternalAntibodies(IIndividualHumanContext* mother, INodeContext* node);

        virtual void ExposeToInfectivity(float = 1.0f, const TransmissionGroupMembership_t* transmissionGroupMembership = NULL);
        virtual void UpdateInfectiousness(float dt);

        // infectivity debugging
        virtual void Drug_Report();
        void malaria_infectivity_report();

        IMalariaSusceptibility* malaria_susceptibility; // now the Individual could have a full pointer to Suscept and Inf, but let's try using limited interface for now

    protected:
        double  m_inv_microliters_blood;   // 1/current blood volume (microliters) based on age

        // mature male and female gametocytes (total)
        int64_t m_male_gametocytes;
        int64_t m_female_gametocytes;

        // ...and by strain
        typedef std::map< StrainIdentity, int64_t > gametocytes_strain_map_t;
        typedef gametocytes_strain_map_t::value_type  gametocytes_strain_t;
        gametocytes_strain_map_t m_male_gametocytes_by_strain;
        gametocytes_strain_map_t m_female_gametocytes_by_strain;

        float m_parasites_detected_by_blood_smear;
        float m_parasites_detected_by_new_diagnostic;
        float m_gametocytes_detected;
        bool  m_clinical_symptoms[ClinicalSymptomsEnum::CLINICAL_SYMPTOMS_COUNT]; // ...or we could store in a bitmask to be even lighter

        IMalariaAntibody* m_CSP_antibody;
        int m_initial_infected_hepatocytes;

        virtual void setupInterventionsContainer();
        virtual Infection* createInfection(suids::suid _suid);

        virtual void ApplyTotalBitingExposure();
        int  CalculateInfectiousBites();

        const SimulationConfig *params() const;
        virtual void PropagateContextToDependents();

    private:
        static float mean_sporozoites_per_bite;
        static float base_sporozoite_survival_fraction;
        static float antibody_csp_killing_threshold;
        static float antibody_csp_killing_invwidth;
        static MalariaModel::Enum malaria_model;

        IndividualHumanMalaria(suids::suid id = suids::nil_suid(), double monte_carlo_weight = 1.0, double initial_age = 0.0, int gender = 0, double initial_poverty = 0.5);
        IndividualHumanMalaria(INodeContext *context);
        virtual IIndividualHumanContext* GetContextPointer() { return (IIndividualHumanContext*)this; }
        void ResetClinicalSymptoms();
        void UpdateGametocyteCounts(float dt);
        void DepositInfectiousnessFromGametocytes();
        void DepositFractionalContagionByStrain(float weight, IVectorInterventionsEffects* ivie, float antigenID, float geneticID);

        virtual bool Configure( const Configuration* config );

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, IndividualHumanMalaria& human, const unsigned int  file_version );
#endif

#if USE_JSON_SERIALIZATION || USE_JSON_MPI
    public:
     // IJsonSerializable Interfaces
     virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const;
     virtual void JDeserialize( IJsonObjectAdapter* root, JSerializer* helper );
#endif
    };
}
