/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/Inria/UBordeaux/UPMC, 2004- .
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/

/**
 * Example of use of the mmg3dls function of the mmg3d library (basic use of
 * level-set discretization option): here the user only provide the level-set
 *
 * \author Charles Dapogny (LJLL, UPMC)
 * \author Pascal Frey (LJLL, UPMC)
 * \author Algiane Froehly (Inria / IMB, Université de Bordeaux)
 * \version 5
 * \copyright GNU Lesser General Public License.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

/** Include the mmg3d library hader file */
// if the header file is in the "include" directory
// #include "libmmg3d.h"
// if the header file is in "include/mmg/mmg3d"
#include "mmg/mmg3d/libmmg3d.h"

int main(int argc,char *argv[]) {
  MMG5_pMesh      mmgMesh;
  MMG5_pSol       mmgLs,mmgMet;
  int             ier;
  char            *inname,*lsname,*outname;

  fprintf(stdout,"  -- TEST MMG3DLS \n");

  if ( argc != 4 ) {
    printf(" Usage: %s meshfile lsfile meshout\n",argv[0]);
    return(1);
  }

  /* Name and path of the mesh files */
  inname = (char *) calloc(strlen(argv[1]) + 1, sizeof(char));
  if ( inname == NULL ) {
    perror("  ## Memory problem: calloc");
    exit(EXIT_FAILURE);
  }
  strcpy(inname,argv[1]);

  lsname = (char *) calloc(strlen(argv[2]) + 1, sizeof(char));
  if ( lsname == NULL ) {
    perror("  ## Memory problem: calloc");
    exit(EXIT_FAILURE);
  }
  strcpy(lsname,argv[2]);


  outname = (char *) calloc(strlen(argv[3]) + 1, sizeof(char));
  if ( outname == NULL ) {
    perror("  ## Memory problem: calloc");
    exit(EXIT_FAILURE);
  }
  strcpy(outname,argv[3]);

  /** 1) Initialisation of mesh and sol structures */
  /* args of InitMesh:
   * MMG5_ARG_start: we start to give the args of a variadic func
   * MMG5_ARG_ppMesh: next arg will be a pointer over a MMG5_pMesh
   * &mmgMesh: pointer to your MMG5_pMesh (that stores your mesh)
   * MMG5_ARG_ppLs: next arg will be a pointer over a MMG5_pSol storing a level-set
   * &mmgLs: pointer to your MMG5_pSol (that stores your level-set)
   */
  mmgMesh = NULL;
  mmgLs   = NULL;
  mmgMet  = NULL;
  MMG3D_Init_mesh(MMG5_ARG_start,
                  MMG5_ARG_ppMesh,&mmgMesh,MMG5_ARG_ppLs,&mmgLs,
                  MMG5_ARG_ppMet,&mmgMet,
                  MMG5_ARG_end);

  /**---------------- Enable the level set discretization --------------------*/
  /* Ask for level set discretization: note that it is important to do this step
   * here because in iso mode, some filters are applied at mesh loading   */
  if ( MMG3D_Set_iparameter(mmgMesh,NULL,MMG3D_IPARAM_iso, 1) != 1 )
    exit(EXIT_FAILURE);

  /* Ask for constant mesh size with edges of length 0.1 */
  if ( MMG3D_Set_dparameter(mmgMesh,NULL,MMG3D_DPARAM_hsiz, 0.5) != 1 )
    exit(EXIT_FAILURE);

  /** 2) Build mesh in MMG5 format */
  /** Two solutions: just use the MMG3D_loadMesh function that will read a .mesh(b)
     file formatted or manually set your mesh using the MMG3D_Set* functions */

  /** with MMG3D_loadMesh function */
  if ( MMG3D_loadMesh(mmgMesh,inname) != 1 )  exit(EXIT_FAILURE);

  /** 3) Build the level-set in MMG5 format */
  /** Two solutions: just use the MMG3D_loadSol function that will read a .sol(b)
      file formatted or manually set your level-set using the MMG3D_Set* functions */
  /** load the level-set With the MMG3D_loadSol function */
  if ( MMG3D_loadSol(mmgMesh,mmgLs,lsname) != 1 )
    exit(EXIT_FAILURE);

  /** 4) (not mandatory): check if the number of given entities match with mesh size */
  if ( MMG3D_Chk_meshData(mmgMesh,mmgLs) != 1 ) exit(EXIT_FAILURE);

  /** 5) (not mandatory): set your global parameters using the
      MMG3D_Set_iparameter and MMG3D_Set_dparameter function
      (resp. for integer parameters and double param)*/


  /**------------------- level set discretization ---------------------------*/

  /** isovalue discretization: as we don't want to impose an input metric we
   * pass NULL instead of the metric structure as function argument */
  ier = MMG3D_mmg3dls(mmgMesh,mmgLs,mmgMet);

  if ( ier == MMG5_STRONGFAILURE ) {
    fprintf(stdout,"BAD ENDING OF MMG3DLS: UNABLE TO SAVE MESH\n");
    return(ier);
  } else if ( ier == MMG5_LOWFAILURE )
    fprintf(stdout,"BAD ENDING OF MMG3DLS\n");

  /* (Not mandatory) Automatically save the mesh */
  if ( MMG3D_saveMesh(mmgMesh,outname) != 1 )
    exit(EXIT_FAILURE);

  /* 9) free the MMG3D5 structures */
  MMG3D_Free_all(MMG5_ARG_start,
                 MMG5_ARG_ppMesh,&mmgMesh,MMG5_ARG_ppLs,&mmgLs,
                 MMG5_ARG_ppMet,&mmgMet,
                 MMG5_ARG_end);

  free(inname);
  inname = NULL;

  free(outname);
  outname = NULL;

  free(lsname);
  outname = NULL;

  return(ier);
}
