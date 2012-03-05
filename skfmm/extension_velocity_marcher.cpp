//extension_velocity_marcher.cpp

#include "extension_velocity_marcher.h"

#include <stdexcept>
#include <iostream>

#include "math.h"

void extensionVelocityMarcher::initalizeFrozen()
{
  //loop over phi to find zero values
  //  and mark them as frozen
  for (int i=0; i<size_; i++)
  {
    if (flag_[i] != Mask && phi_[i]==0.0)
    {
      flag_[i]=Frozen;
      distance_[i]=0.0;
      f_ext_[i]=speed_[i];
    }
  }
  //loop over all of phi and for each point check each direction
  //  to see if we cross the zero level set
  //     if so calculate the minimum distance to the zero level set
  //     mark as frozen.
  for (int i=0; i<size_; i++)
  if (flag_[i] == Far)
  {
    double ldistance[MaximumDimension];
    double lspeed[MaximumDimension];
    bool borders=false;
    for (int dim=0; dim<dim_; dim++)
    {
      ldistance[dim]=0;
      lspeed[dim]=0;
      for (int j=-1; j<2; j+=2) // each direction
      {
        int naddr = _getN(i,dim,j,Mask);
        if (naddr!=-1 && phi_[i] * phi_[naddr]<0)
        {
          // this cell and neighbor span the zero level set.
          borders=true;
          //calculate the distance to the zero level set.
          double d = dx_[dim]*phi_[i]/(phi_[i]-phi_[naddr]);
          if (ldistance[dim]==0 || ldistance[dim]>d)
          {
            ldistance[dim] = d;
            lspeed[dim] = speed_[naddr];
          }
        }
      } // for each direction
    } // for each dimension
    if (borders)
    {
      double numerator = 0.0;
      double denominator = 0.0;
      for (int dim=0; dim<dim_; dim++)
      {
        numerator += lspeed[dim]/pow(ldistance[dim],2);
        denominator += 1/pow(ldistance[dim],2);
      }
      f_ext_[i] = numerator/denominator;

      double dsum = 0;
      for (int dim=0; dim<dim_; dim++)
        if (ldistance[dim]>0) dsum += 1/ldistance[dim]/ldistance[dim];
      if (phi_[i]<0)
        distance_[i] = -sqrt(1/dsum);
      else distance_[i] = sqrt(1/dsum);
      flag_[i]=Frozen;
    }
  }// for each point in the far field
}

double extensionVelocityMarcher::updatePoint(int i)
{
   // get the updated position of the point
  double phi_i = distanceMarcher::updatePoint(i);
  // find f_ext where grad f_ext . grad phi = 0
  // as described in Adalsteinsson and Sethian

  // technically we do not need to calculate this extension velocity
  // until the point is frozen.

  double ldistance[MaximumDimension];
  double lspeed[MaximumDimension];

  double numerator = 0.0;
  double denominator = 0.0;

  for (int dim=0; dim<dim_; dim++)
  {
    lspeed[dim]=0;
    ldistance[dim]=0;
    for (int j=-1; j<2; j+=2) // each direction
    {
      int naddr = _getN(i,dim,j,Mask);
      if (naddr!=-1 && flag_[naddr]==Frozen)
      {
        //determine which direction, in this dimension, is nearest to
        //the front. Calculate the distance to front in this direction
        double d = phi_i - phi_[naddr];
        if (ldistance[dim]==0 || ldistance[dim]>d)
        {
          ldistance[dim] = d;
          lspeed[dim] = speed_[naddr];
        }
      }
    } // for each direction
  } // for each dimension

  for (int dim=0; dim<dim_; dim++)
  {
    numerator += ldistance[dim]*lspeed[dim]*idx2_[dim];
    denominator += ldistance[dim]*idx2_[dim];
  }

  if (denominator != 0.0)
  {
    f_ext_[i] = numerator/denominator;
  }
  else
  {
    throw std::runtime_error("div by zero error in extension velocity");
  }

  return phi_i;
}

void extensionVelocityMarcher::cleanUp()
{
  for (int i=0; i<size_; i++)
  {
    if (flag_[i] == Mask) f_ext_[i] = maxDouble;
    if (flag_[i] == Far)  f_ext_[i] = maxDouble;
  }
}
