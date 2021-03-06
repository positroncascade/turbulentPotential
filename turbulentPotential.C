/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "turbulentPotential.H"
#include "addToRunTimeSelectionTable.H"
#include "backwardsCompatibilityWallFunctions.H"
#include "components.H"
#include "fvCFD.H"
#include "volFields.H"
#include "wallDist.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Hello

namespace Foam
{
namespace incompressible
{
namespace RASModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(turbulentPotential, 0);
addToRunTimeSelectionTable(RASModel, turbulentPotential, dictionary);

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

tmp<volScalarField> turbulentPotential::Ts() const
{ 
	if(tslimiter_ == "true")
	{
        return max(k_/(epsilon_ + epsilonSmall_), 6.0*sqrt(nu()/(epsilon_ + epsilonSmall_)));
	}
	
    return ((k_+k0_)/(epsilon_ + epsilonSmall_));
}


tmp<volScalarField> turbulentPotential::Ls() const
{
	
	volScalarField trueL = pow(k_+k0_, 1.5)/(epsilon_ + epsilonSmall_);
	
	if(lslimiter_ == "true")
	{
		return cL1_*max(pow(k_+k0_, 1.5)/(epsilon_ + epsilonSmall_),cL2_*pow(pow3(nu())/(epsilon_ + epsilonSmall_),0.25));
	}
	
	//Info << "Max trueL: " << gMax(trueL) << " Min trueL: " << gMin(trueL) << endl;
	//Info << "Dims: " << trueL.dimensions() << endl;
	
	return pow(k_+k0_, 1.5)/(epsilon_ + epsilonSmall_);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

turbulentPotential::turbulentPotential
(
    const volVectorField& U,
    const surfaceScalarField& phi,
    transportModel& lamTransportModel
)
:
    RASModel(typeName, U, phi, lamTransportModel),


    cEp1_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cEp1",
            coeffDict_,
            1.45
        )
    ),
    cEp2con_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cEp2con",
            coeffDict_,
            1.83
        )
    ),
    cEp3_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cEp3",
            coeffDict_,
            0.15
        )
    ),
    cD1_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cD1",
       	    coeffDict_,
            0.5
        )
    ),
    cD2_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cD2",
       	    coeffDict_,
            0.33
        )
    ),
    cD3_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cD3",
       	    coeffDict_,
            3.0
        )
    ),
    cD4_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cD4",
       	    coeffDict_,
            2.4
        )
    ),
    cP1_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cP1",
            coeffDict_,
            2.0
        )
    ),
    cP2_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cP2",
            coeffDict_,
            0.6
        )
    ),
    cP3_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cP3",
            coeffDict_,
            0.42
        )
    ),
    cP4_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cP4",
            coeffDict_,
            0.85714
        )
    ),
    cL1_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cL1",
            coeffDict_,
            0.36
        )
    ),
    cL2_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cL2",
            coeffDict_,
            85.0
        )
    ),
    cMu_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cMu",
            coeffDict_,
            0.21
        )
    ),
    betaK_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "betaK",
            coeffDict_,
            0.09
        )
    ),
    cT_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cT",
            coeffDict_,
            0.02
        )
    ),
    cA_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cA",
            coeffDict_,
            1.0
        )
    ),
    cEhmM_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cEhmM",
            coeffDict_,
            10.0
        )
    ),
	cNF_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cNF",
            coeffDict_,
            1.0
        )
    ),
	pMix_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "pMix",
            coeffDict_,
            0.4
        )
    ),
	cPrK_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cPrK",
            coeffDict_,
            0.6
        )
    ),
	cPrP_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cPrP",
            coeffDict_,
            1.0
        )
    ),
	cNL_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "cNL",
            coeffDict_,
            0.00001
        )
    ),
	nutRatMax_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "nutRatMax",
            coeffDict_,
            1.0e5
        )
    ),
    sigmaK_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "sigmaK",
            coeffDict_,
            1.0
        )
    ),
    sigmaEps_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "sigmaEps",
            coeffDict_,
            0.833
        )
    ),
    sigmaPhi_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "sigmaPhi",
            coeffDict_,
            0.33
        )
    ),
    sigmaPsi_
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "sigmaPsi",
            coeffDict_,
            1.0
        )
    ),
	prodType_
	(
		dimensionedScalar::lookupOrAddToDict
		(
			"prodType",
			coeffDict_,
			1.0
		)
	),
	dampType_
	(
		dimensionedScalar::lookupOrAddToDict
		(
			"dampType",
			coeffDict_,
			1.0
		)
	),
	cEp1Type_
	(
		dimensionedScalar::lookupOrAddToDict
		(
			"cEp1Type",
			coeffDict_,
			1.0
		)
	),
	gType_
	(
		dimensionedScalar::lookupOrAddToDict
		(
			"gType",
			coeffDict_,
			1.0
		)
	),
	
   solveK_
   (
       coeffDict_.lookup("solveK")
   ),

   solveEps_
   (
       coeffDict_.lookup("solveEps")
   ),

   solvePsi_
   (
       coeffDict_.lookup("solvePsi")
   ),

   solvePhi_
   (
       coeffDict_.lookup("solvePhi")
   ),

   solveNut_
   (
       coeffDict_.lookup("solveNut")
   ),

   eqncEp1_
   (
       coeffDict_.lookup("eqncEp1")
   ),
   
   eqncEp2_
   (
       coeffDict_.lookup("eqncEp2")
   ),

   eqnEpsHat_
   (
       coeffDict_.lookup("eqnEpsHat")
   ),

   debugWrite_
   (
       coeffDict_.lookup("debugWrite")
   ),
   tslimiter_
   (
       coeffDict_.lookup("tslimiter")
   ),
   lslimiter_
   (
       coeffDict_.lookup("lslimiter")
   ),
   eqncMu_
   (
       coeffDict_.lookup("eqncMu")
   ),
   nutReal_
   (
       coeffDict_.lookup("nutReal")
   ),
   y_
   (
   mesh_
   ),
    
	k_
    (
        IOobject
        (
            "k",
            runTime_.timeName(),
            U_.db(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh_
    ),
    
	gradk_
    (
        IOobject
        (
            "gradk",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (fvc::grad(k_))
    ),
    
	epsilon_
    (
        IOobject
        (
            "epsilon",
            runTime_.timeName(),
            U_.db(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh_
    ),
    
	nut_
    (
        IOobject
        (
            "nut",
            runTime_.timeName(),
            U_.db(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh_
    ),
    
	nutNorm_
    (
        IOobject
        (
            "nutNorm",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (nut_/max(nut_))
    ),
    
	tpphi_
    (
        IOobject
        (
            "tpphi",
            runTime_.timeName(),
            U_.db(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh_
    ),
    
	tpphiSqrt_
    (
        IOobject
        (
            "tpphiSqrt",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (sqrt(tpphi_))
    ),
	
	vorticity_
    (
        IOobject
        (
            "vorticity",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        (fvc::curl(U_))
    ),
    
	tppsi_
    (
        IOobject
        (
            "tppsi",
            runTime_.timeName(),
            U_.db(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh_
    ),
    
	uGrad_
    (
        IOobject
        (
            "uGrad",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        (fvc::grad(U_))
    ),
	
	epsHat_
    (
        IOobject
        (
            "epsHat",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        (epsilon_/(k_ + k0_))
    ),
    
	kSqrt_
    (
        IOobject
        (
            "kSqrt",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (sqrt(k_))
    ),
	
	alpha_
    (
        IOobject
        (
            "alpha",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (1.0/(1.0 + 1.5*tpphi_))
    ),
	
	gamma_
    (
        IOobject
        (
            "gamma",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        (1.0/(1.0 + 0.667*nut_/nu()))
    ),
	
	phiSqrt_
    (
        IOobject
        (
            "phiSqrt",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (sqrt(tpphi_*k_))
    ),
    
	gradkSqrt_
    (
        IOobject
        (
            "gradkSqrt",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (fvc::grad(kSqrt_))
    ),
    
	cEp2_
    (
        IOobject
        (
            "cEp2",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (cEp2con_ - 0.16*exp(-0.1*sqr(k_)/(nu()*epsilon_)))
    ),
    
	tpProd_
    (
        IOobject
        (
            "tpProd",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        ((2*nut_*magSqr(symm(fvc::grad(U_)))/k_))
    ),
    
	cP1eqn_
    (
        IOobject
        (
            "cP1eqn",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (2.0*(0.5+0.5*((tpProd_*k_)/epsilon_)))
    ),
    
	gradTpphi_
    (
        IOobject
        (
            "gradTpphi",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (fvc::grad(tpphi_))
    ),
    
	gradTppsi_
    (
        IOobject
        (
            "gradTppsi",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (fvc::grad(tppsi_))
    ),
    
	tpProdSqr_
    (
        IOobject
        (
            "tpProdSqr",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (sqr(tppsi_ & vorticity_))
    ),
    
	tpProd3d_
    (
        IOobject
        (
            "tpProd3d",
            runTime_.timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (mag(psiReal() ^ vorticity_))
    )
{

    Info<< "Made it past constructors " << endl;
	
	
    //*************************************//	
    // Eddy viscosity - diffusion only
    //*************************************//
    if(solveNut_ == "true")
    {
		nut_ = cMu_*k_*tpphi_*Ts();	
        nut_ = min(nut_,nutRatMax_*nu());        
        nut_.correctBoundaryConditions();
        bound(nut_,dimensionedScalar("minNut", nut_.dimensions(), SMALL));       
    }
	
	
    //*************************************//	
    // Epsilon-hat
    //*************************************//   
    epsHat_ = epsilon_/(k_ + (cEhmM_*nu()*mag(gradkSqrt_)));
    bound(epsHat_,dimensionedScalar("minEpsHat", epsHat_.dimensions(), SMALL));
	
	
    Info<< "solveK is: " <<solveK_ <<endl;
    Info<< "solveEps is: " <<solveEps_ <<endl;
    Info<< "solvePhi is: " <<solvePhi_ <<endl;
    Info<< "solvePsi is: " <<solvePsi_ <<endl;

    printCoeffs();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Not used but necessary for RAS Model
tmp<volSymmTensorField> turbulentPotential::R() const
{
    return tmp<volSymmTensorField>
    (
        new volSymmTensorField
        (
            IOobject
            (
                "R",
                runTime_.timeName(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            ((2.0/3.0)*I)*k_ - nut_*twoSymm(fvc::grad(U_))
        )
    );
}

// Not used but necessary for RAS Model
tmp<volSymmTensorField> turbulentPotential::devReff() const
{
    return tmp<volSymmTensorField>
    (
        new volSymmTensorField
        (
            IOobject
            (
                "devRhoReff",
                runTime_.timeName(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            -nuEff()*dev(twoSymm(fvc::grad(U_)))
        )
    );
}

// Term that is directly added to the momentum equation
tmp<fvVectorMatrix> turbulentPotential::divDevReff() const
{
    return
    (
       fvc::grad(phiReal())
     + fvc::curl(psiReal())
     + fvc::laplacian(nut_, U_, "laplacian(nuEff,U_)")
     - fvm::laplacian(nuEff(), U_)
    );
}


bool turbulentPotential::read()
{
    if (RASModel::read())
    {
        cEp1_.readIfPresent(coeffDict());
        cEp2con_.readIfPresent(coeffDict());
        cEp3_.readIfPresent(coeffDict());
        cP1_.readIfPresent(coeffDict());
        cP2_.readIfPresent(coeffDict());
        cP3_.readIfPresent(coeffDict());
		cP4_.readIfPresent(coeffDict());
		cL1_.readIfPresent(coeffDict());
		cL2_.readIfPresent(coeffDict());
        cMu_.readIfPresent(coeffDict());
		cEhmM_.readIfPresent(coeffDict());
		cPrK_.readIfPresent(coeffDict());
		cPrP_.readIfPresent(coeffDict());
		cD1_.readIfPresent(coeffDict());
		cD2_.readIfPresent(coeffDict());
		cD3_.readIfPresent(coeffDict());
		cD4_.readIfPresent(coeffDict());
		cT_.readIfPresent(coeffDict());
		cA_.readIfPresent(coeffDict());
		cNL_.readIfPresent(coeffDict());
		cNF_.readIfPresent(coeffDict());
		sigmaK_.readIfPresent(coeffDict());
        sigmaEps_.readIfPresent(coeffDict());
        sigmaPhi_.readIfPresent(coeffDict());
		sigmaPsi_.readIfPresent(coeffDict());
		prodType_.readIfPresent(coeffDict());
		cEp1Type_.readIfPresent(coeffDict());
		gType_.readIfPresent(coeffDict());

        return true;
    }
    else
    {
        return false;
    }
}


void turbulentPotential::correct()
{

    //**********************************************//	
    // Bounding values not already defined by model
    //**********************************************//
	
    const dimensionedScalar eH0("minEpsHat", epsHat_.dimensions(), ROOTVSMALL);
	const dimensionedScalar nut0("minNut", nut_.dimensions(), ROOTVSMALL);
	const dimensionedScalar nutSmall("smallNut", nut_.dimensions(), SMALL);
	const dimensionedScalar tph0("minTpphi", tpphi_.dimensions(), ROOTVSMALL);
	const dimensionedScalar L0("lMin", dimensionSet(0,1,0,0,0,0,0), ROOTVSMALL);

    if (mesh_.changing())
    {
        y_.correct();
        bound(k_, k0_);
        bound(epsilon_, epsilonSmall_);
		bound(tpphi_,tph0);
		bound(nut_,nut0);
    }
	
	
    RASModel::correct();

	
    if (!turbulence_)
    {
        return;
    }
	
	
    //*************************************//	
    // Timestep - for use in elliptic switch
    //*************************************//
	
    dimensionedScalar cTime = U_.mesh().time().value();
	word sMMdebug = runTime_.controlDict().lookup("showMaxMin");


    //*************************************//	
    // Vorticity and Gradient
    //*************************************//
    
	vorticity_ = fvc::curl(U_);
	uGrad_ = fvc::grad(U_);
	


    //*************************************//	
    // Length and Time Scales
    //*************************************//	
	
	const volScalarField L("Length",Ls());
	const volScalarField L2("Lsqr",sqr(L));
	const volScalarField T("Time",Ts());	
		
	const volScalarField nutPsi("nutPsi", mag(tppsi_)*k_/(mag(vorticity_) + (cNL_/Ts())));

	
	//*************************************//	
    // Misc Terms
    //*************************************//

	const volVectorField gradPhi_("gradPhi", fvc::grad(phiReal()));		
	const volScalarField gradgradPhi_("gradgradPhi", fvc::laplacian(DphiEff(),phiReal()));

	tpphiSqrt_ = sqrt(tpphi_ + ROOTVSMALL);
	const volVectorField gradTpphiSqrt("gradTpphiSqrt",fvc::grad(tpphiSqrt_));

	kSqrt_ = sqrt(mag(k_)+k0_);
    bound(kSqrt_,dimensionedScalar("minKsqrt", kSqrt_.dimensions(), sqrt(ROOTVSMALL)));

    gradk_ = fvc::grad(k_);
    gradkSqrt_ = fvc::grad(kSqrt_);
	
    gradTpphi_ = fvc::grad(tpphi_);
    phiSqrt_ = sqrt(tpphi_*k_ + k0_);
	const volVectorField gradPhiSqrt_("gradPhiSqrt",fvc::grad(phiSqrt_));	
	
	
    //*************************************//	
    // K Production
    //*************************************//
 
	const volScalarField S2 = 2*magSqr(dev(symm(uGrad_)));
	const volScalarField magS = sqrt(S2);
	volScalarField G("RASModel::G", nut_*S2); 
	volScalarField GdK("GdK", G/(k_ + k0_));
	const volScalarField Gnut("Gnut", nut_*S2);
	

	if(prodType_.value() == 1.0){
		Info<< "Using psi-vorticity production term" <<endl;
		tpProd_ = mag(tppsi_ & vorticity_);
		G = tpProd_*k_;
		GdK = tpProd_;	
	} else if(prodType_.value() == 2.0){
		Info<< "Using strain production term" <<endl;
		tpProd_ = GdK;
	} else if(prodType_.value() == 3.0){
		Info<< "Using mixed 3 production term" <<endl;
		tpProd_ = alpha_*mag(tppsi_ & vorticity_) + pMix_*(1.0-alpha_)*cPrK_*magS + (1.0 - pMix_)*(1.0 - alpha_)*cPrP_*tpphi_*magS;
		G = tpProd_*k_;
		GdK = tpProd_;	
    } else if(prodType_.value() == 4.0){
		Info<< "Using mixed 4 production term" <<endl;
		tpProd_ = alpha_*mag(tppsi_ & vorticity_) + 0.94*(1.0-alpha_)*GdK;
		G = tpProd_*k_;
		GdK = tpProd_;	
    } else if(prodType_.value() == 5.0){
		Info<< "Using mixed 5 production term" <<endl;
		G = alpha_*mag((tppsi_*k_) & vorticity_) + 0.27*(1.0-alpha_)*(k_ - 1.5*nut_*mag(gradPhiSqrt_))*magS;
		tpProd_ = G/(k_+k0_);
		GdK = tpProd_;			
	} else{
		Info<< "Using psi-vorticity production term" <<endl;
		tpProd_ = mag(tppsi_ & vorticity_);
		G = tpProd_*k_;
		GdK = tpProd_;		
	}
    
	tpProdSqr_ = sqr(tpProd_);
	tpProd3d_ = mag(psiReal() ^ vorticity_);
	
	const volScalarField pOD = G/epsilon_; 
	
	
	

	//*************************************//	
    // Update Alpha
    //*************************************//
    
	alpha_ = 1.0/(1.0 + 1.5*tpphi_);

	if(gType_.value() == 1.0){
		gamma_ = 1.0/(1.0 + 0.05*(1.5 - alpha_)*(1.0 + nut_/nu()));
	}
	
	if(gType_.value() == 2.0){
		gamma_ = 1.0/(1.0 + 0.667*(1.0 - alpha_)*pow(SMALL + nut_/nu(), 0.5));
	}

	if(gType_.value() == 3.0){
		gamma_ = 1.0/(1.0 + 0.12*(1.0 - alpha_)*(nut_/nu()));
	}
	
	if(gType_.value() == 4.0){
		gamma_ = 1.0/(1.0 + 0.08*pow(tpphi_+SMALL,0.5)*(nut_/nu()));
	}
	
    //*************************************//
    //Dissipation equation
    //*************************************//
    volScalarField cEp1eqn("cEp1eqn",(cEp1_*(tpphi_/tpphi_)));

	if(eqncEp1_ == "true")
	{
		//cEp1eqn = min(2.0*(tpphi_/tpphi_),(cEp1_)*(1.0+0.09*(alpha_-0.42))); 
		//cEp1eqn = 4.0*1.4*(0.5*alpha_*(0.67-tpphi_) + (1.0-alpha_)*tpphi_);	
        //cEp1eqn = (cEp1_-0.07)*(1.0 + 0.08*alpha_);	
        //cEp1eqn = cEp1_*(1.0 + 0.07*sqr(gamma_));	
		//cEp1eqn = cEp1_*(0.97 + 0.11*(2.0*alpha_-1.0));
        //cEp1eqn = 2.91-cEp1_*(0.963 + 0.2*mag(tppsi_));	
        //cEp1eqn = cEp1_ + 0.033*(2.0*alpha_-1.0);		
		//cEp1eqn = cEp2con_ - exp(-0.75*pow((1.0/tpphi_) + SMALL, 0.25));
		//cEp1eqn = cEp1_ + 0.12*pow((2.0*alpha_-1.0),2.0);
		
		if(cEp1Type_.value() == 1.0){
			cEp1eqn = cEp1_ + (0.06 - 0.18*mag(tppsi_));
		}
		
		if(cEp1Type_.value() == 2.0){
			cEp1eqn = cEp1_ + 0.067*(2.0*alpha_-1.0);
		}
		
		if(cEp1Type_.value() == 3.0){
			cEp1eqn = cEp1_ + 0.033*sqrt(2.0*alpha_-1.0);
		}
		
		if(cEp1Type_.value() == 4.0){
			cEp1eqn = 1.4*(1.0 + 0.012*(k_/(tpphi_*k_ + 0.01*k_)));
		}
		
		if(cEp1Type_.value() == 5.0){
			cEp1eqn = 1.4*(1.0 + 0.015*(k_/(tpphi_*k_ + 0.03*k_)));
		}
		
		if(cEp1Type_.value() == 6.0){
			cEp1eqn = 1.4*(1.0 + 0.022*(k_/(tpphi_*k_ + 0.048*k_)));
		}
		
		if(cEp1Type_.value() == 7.0){
			cEp1eqn = cEp1_*(0.985 + 0.01*(k_/(tpphi_*k_ + 0.02*k_)));
		}
		
		if(cEp1Type_.value() == 8.0){
			cEp1eqn = cEp1_*(0.994 + 0.0033*(k_/(tpphi_*k_ + 0.015*k_)));
		}
		
		if(cEp1Type_.value() == 9.0){
			cEp1eqn = cEp1_*(0.995 + 0.002*(k_/(tpphi_*k_ + 0.02*k_)));
		}
		
		if(cEp1Type_.value() == 10.0){
			cEp1eqn = cEp1_*(0.995 + 0.0045*(k_/(tpphi_*k_ + 0.015*k_)));
		}
		
		if(cEp1Type_.value() == 11.0){
			cEp1eqn = cEp1_*(0.985 + 0.008*(k_/(tpphi_*k_ + 0.02*k_)));
		}
	}
	
    if(eqncEp2_ == "true")
    {
        cEp2_ = cEp2con_ - 0.16*exp(-0.25*sqr(k_)/(nu()*(epsilon_ + epsilonSmall_)));
    }
    else
    {
        cEp2_ =  cEp2con_;
    }
	

    tmp<fvScalarMatrix> epsEqn   
    (
        fvm::ddt(epsilon_)
      + fvm::div(phi_, epsilon_)
      + fvm::SuSp(-fvc::div(phi_), epsilon_)
      - fvm::laplacian(DepsilonEff(), epsilon_)
     ==
       cEp1eqn*G*epsHat_
     - fvm::Sp(cEp2_*epsHat_,epsilon_)
    );

    if(solveEps_ == "true")
    {
    epsEqn().relax();
    solve(epsEqn);
    bound(epsilon_,epsilonSmall_);
    }
	
	
	
	
    //*************************************//
    // Turbulent kinetic energy equation
    //*************************************//
    
    tmp<fvScalarMatrix> kEqn
    (

        fvm::ddt(k_)
      + fvm::div(phi_, k_)
      + fvm::SuSp(-fvc::div(phi_), k_)
      - fvm::laplacian(DkEff(), k_)
     ==
        G
      - fvm::Sp(epsilon_/(k_+k0_),k_)
    );


    if(solveK_ == "true")
    {
    kEqn().relax();
    solve(kEqn);
    bound(k_,k0_);
    }
	
	
	
	
	//*************************************//	
    // Epsilon-hat
    //*************************************//
    
    epsHat_ = epsilon_/(k_ + (cEhmM_*nu()*mag(gradkSqrt_)));
    bound(epsHat_,eH0);

	
	
	
    //*************************************//
    // Fix for separated flows 
    //*************************************// 	

	volScalarField cTexp("cTexp", cT_*(1.0-gamma_)*sqrt((((nu()/100.0)+nut_)/nu())));
	
    volScalarField transPhi("transPhi", cTexp*cA_*((2.0/3.0) - tpphi_)*tpProd_);	
	volVectorField transPsi("transPsi", cTexp*((1.0 - alpha_)*vorticity_ - cA_*tppsi_*tpProd_));	
	
	
	
    //*************************************//
    // Pressure strain damping
    //*************************************// 
	volScalarField phiDamp("phiDamp", nut_/(nut_ + cNF_*nu()));
	
	if(dampType_.value() == 1.0){
		Info<< "Using nutFrac damping" << endl;
		phiDamp = nut_/(nut_ + cNF_*nu());
	}
	
	if(dampType_.value() == 2.0){
		Info<< "Using exponent damping" << endl;
		phiDamp = cD3_*alpha_*pow(1.0 - exp(-cD4_*tpphi_), 2.0);
	}

	if(dampType_.value() == 3.0){
		Info<< "Using exponent (no alpha) damping" << endl;
		phiDamp = cD3_*(1.0 - exp(-cD4_*(1.0-alpha_)));
	}
	
	if(dampType_.value() == 4.0){
		Info<< "Using 1-alpha ^3/2 damping" << endl;
		phiDamp = 4.1*pow(1.0-alpha_,1.5);
	}   

	if(dampType_.value() == 5.0){
		Info<< "Using alpha(1-alpha)^2 damping" << endl;
		phiDamp = cD2_*alpha_*pow(1.0-alpha_,2.0);
	}	
	
	if(dampType_.value() == 6.0){
		Info<< "Using nutFrac alpha damping" << endl;
		phiDamp = nut_/(nut_ + cNF_*(2.0*alpha_-1.0)*nu());
	}
	
	if(dampType_.value() == 7.0){
		Info<< "Using nutFrac sqrt(alpha) gamma damping" << endl;
		phiDamp = nut_/(nut_ + cNF_*sqrt(alpha_ + SMALL)*gamma_*nu());
	}
	
	if(dampType_.value() == 8.0){
		Info<< "Using nutFrac sqrt(2alpha-1) gamma^2 damping" << endl;
		phiDamp = nut_/(nut_ + cNF_*sqrt(2.0*alpha_-1.0 + SMALL)*sqr(gamma_)*nu());
	}
	
	if(dampType_.value() == 9.0){
		Info<< "Using nutFrac gamma^1.5 damping" << endl;
		phiDamp = nut_/(nut_ + cNF_*pow(gamma_, 1.5)*nu());
	}
	
	if(dampType_.value() == 10.0){
		Info<< "Using nutFrac gamma damping" << endl;
		phiDamp = nut_/(nut_ + cNF_*gamma_*nu());
	}
	
	if(dampType_.value() == 11.0){
		Info<< "Using van driest type damping" << endl;	
		phiDamp = 0.9*sqr(1.0-exp(-0.12*(nut_/nu())));
	}
	
	if(dampType_.value() == 12.0){
		Info<< "Using van driest psi type damping" << endl;	
		phiDamp = (1.0-exp(-0.085*(1.0-0.25*gamma_)*(nutPsi/nu())));
	}
	
	if(dampType_.value() == 13.0){
		Info<< "Using 1-gamma^5/3 damping" << endl;	
		phiDamp = 1.0-pow(gamma_,5.0/3.0);  
	}
	
	if(dampType_.value() == 14.0){
		Info<< "Using 1-gamma^5/3 damping" << endl;	
		phiDamp = nut_/(nut_ + cNF_*((2.0*alpha_-1.0)+0.4*mag(tppsi_))*nu());  
	}
	
	if(dampType_.value() == 15.0){
		Info<< "Using 1-alpha nutFrac damping" << endl;	
		phiDamp = nut_/(nut_ + cNF_*nu());  
	}
	
	//*************************************//
    // Phi/K equation 
    //*************************************//
	cP1eqn_ = cP1_ - gamma_;
	volScalarField bphi("bphi", ((2.0/3.0) - tpphi_));

    tmp<fvScalarMatrix> tpphiEqn
    ( 
        fvm::ddt(tpphi_)
      + fvm::div(phi_, tpphi_)
      + fvm::SuSp(-fvc::div(phi_), tpphi_)
      - fvm::laplacian(DphiEff(), tpphi_) 
      == 
	  // Pressure Strain Slow + Fast
	    (cP1_ + 0.8*pOD)*phiDamp*(1.0-gamma_)*bphi/T
	  + cP2_*(1.0 + 0.1*gamma_)*tpProd_*tpphi_

	  // From K eqn
      - fvm::Sp(GdK,tpphi_)
	  
	  // Dissipation
	  - (1.0-gamma_)*bphi/T
	  
	  // Transition
      + transPhi
    );

    if(solvePhi_ == "true")
    {
    tpphiEqn().relax();
    solve(tpphiEqn);
    bound(tpphi_,tph0);
    }

	


    //*************************************//   
    // Psi Specific Constants
    //*************************************//
	//volVectorField psiDisWall("psiDisWall", cD1_*gamma_*(2.0*alpha_-1.0)*tpphi_*vorticity_);	
	volVectorField psiDisWall("psiDisWall", cD1_*gamma_*(1.0-phiDamp)*tpphi_*vorticity_);
	
    //*************************************//   
    // Psi Equation
    //*************************************//
    
    tmp<fvVectorMatrix> tppsiEqn
    (
        fvm::ddt(tppsi_)
      + fvm::div(phi_, tppsi_)
      + fvm::Sp(-fvc::div(phi_), tppsi_)
      - fvm::laplacian(DpsiEff(), tppsi_)

      ==

	  // Production
	    tpphi_*vorticity_
		
	  // Slow Pressure Strain
      - fvm::Sp((cP1_ + 0.8*pOD)*phiDamp*(1.0-gamma_)/T,tppsi_)

	  // Fast Pressure Strain
	  - cP2_*tpphi_*vorticity_
	  + cP2_*(1.0 + 0.1*gamma_)*tpProd_*tppsi_
	  - cP3_*(1.0-alpha_)*vorticity_
	  
	  // From K Equation
      - fvm::Sp(tpProd_,tppsi_)
	  
	  // Dissipation 
	  + (1.0-gamma_)*tppsi_/T
	  + psiDisWall 

	  // Transition Term
      + transPsi
    );

    if(solvePsi_ == "true") 
    {
    tppsiEqn().relax();
    solve(tppsiEqn);
    }
	
    // Re-calculate psi/k gradient
    gradTppsi_ = fvc::grad(tppsi_);
	


	//*************************************//
    // Calculate eddy viscosity
    //*************************************//
	volScalarField tppsiSqr("tppsiSqr", (tppsi_ & tppsi_)); 

	volScalarField cMuEqn("cMuEqn", (0.16 + 0.67*gamma_*mag(tppsi_)*(exp(-0.333*pOD))));
	//volScalarField cMuEqn("cMuEqn", (0.16 + 0.12*(1.0 - (exp(-1.67*gamma_))));
	 
    if(solveNut_ == "true")     
    {       
		if(eqncMu_ == "true"){
			nut_ = cMuEqn*k_*tpphi_*T;
		}else{
			if(nutReal_ == "true"){
				nut_ = min(cMu_*k_*tpphi_*T, max(nutPsi, (0.6/sqrt(3.0))*k_/magS));
			}else{
				nut_ = cMu_*k_*tpphi_*T;
			}
		}  
		nut_ = min(nut_,nutRatMax_*nu()); 
		nut_.correctBoundaryConditions();
        bound(nut_,nut0); 
    }

	
    //*************************************//   
    // Output some max values
    //*************************************//
	
	if(sMMdebug == "true")
	{    
    volScalarField phiActual("phiActual",tpphi_*k_);
	volVectorField psiActual("psiActual",tppsi_*k_);
	volScalarField meanUz("meanUz",U_.component(2));
	volScalarField uTauSquared((nu() + nut_)*vorticity_.component(2));
	volVectorField tpphiVort(tpphi_*vorticity_);
	
	Info<< "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << endl; 
	Info<< "Max cEp1: " << gMax(cEp1eqn) << " Min cEp1: " << gMin(cEp1eqn) << endl;  
	Info<< "Max Epsilon: " << gMax(epsilon_) << " Max G: " << gMax(G) << " Max Gnut: " << gMax(Gnut) <<endl;
	Info<< "Max nut: " << gMax(nut_) << " Max K: " << gMax(k_) << " Max Phi: " << gMax(phiActual) <<endl;
    Info<< "Max Psi: " << gMax(psiActual) << " Min Psi: " << gMin(psiActual)  <<endl;
    Info<< "Max uTauSquared: " << gMax(uTauSquared) <<endl; 
	Info<< "Max vorticity: " << gMax(vorticity_) << " Min vorticity: " << gMin(vorticity_) <<endl;
	Info<< "Max Uz: " << gMax(meanUz) << " Min Uz: " << gMin(meanUz) << endl;
	Info<< "Max gamma: " << gMax(gamma_) << " Min gamma: " << gMin(gamma_) << endl;
	Info<< "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << endl; 
	}

}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace incompressible
} // End namespace Foam

// ************************************************************************* //
