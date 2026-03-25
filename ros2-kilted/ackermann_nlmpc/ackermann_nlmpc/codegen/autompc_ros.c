/* Copyright (C) 2025, Georg Schildbach.
 * -------------------------------------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <http://www.gnu.org/licenses/>.
 * -------------------------------------------------------------------------------------------------
 * Your feedback or questions or bug reports are highly welcome! Contact me if you are interested in
 * using this product commercially. Please address all correspondance to: gschildbach(at)gmail.com
 * -------------------------------------------------------------------------------------------------
 */
 
#define macheps FLT_EPSILON
 
#include "stdint.h"
#include "float.h"
#include "math.h"
 
 
static void gsrefgen(const double *t, const double *x0, const double *Traj, const double *amin, const double *amax, uint8_t *drivmode, double *Ref, int64_t *vi, double *vd)
{
    static double  Tt = -1e15;
    static int64_t trajsegm = 0;
    vi[6] = (int64_t)(Traj[5]+0.5);
    vi[7] = (int64_t)(Traj[4]+0.5);
    if ((trajsegm<vi[6]) && (trajsegm<2500))
    {
        *drivmode = (uint8_t)(Traj[14+trajsegm*11]+0.5);
    }
    else
    {
        *drivmode = 0;
    }
    if (Tt+(1e-10) < Traj[0])
    {
        vi[3] = 1;
        Tt = Traj[0];
        trajsegm = 0;
        *drivmode = (uint8_t)(Traj[14]+0.5);
    }
    else
    {
        vi[3] = 0;
        if ((trajsegm<vi[6]) && (trajsegm<2500))
        {
            if ((*drivmode) > 0)
            {
                if (vi[7] < 2)
                {
                    vi[1] = trajsegm;
                    for (vi[0]=trajsegm-1; ((vi[0]>=vi[1]-10) && (vi[0]>=0)); vi[0]--) {
                        if ((uint8_t)(Traj[14+vi[0]*11]+0.5) == (*drivmode))
                        {
                            trajsegm = vi[0];
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    vi[1] = trajsegm;
                    for (vi[0]=0; vi[0]<10; vi[0]++) {
                        vi[1]--;
                        while (vi[1] < 0)
                        {
                            vi[1] = vi[1] + vi[6];
                        }
                        if ((uint8_t)(Traj[14+vi[1]*11]+0.5) == (*drivmode))
                        {
                            trajsegm = vi[1];
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                while ((trajsegm<vi[6]) && (trajsegm<2500))
                {
                    if (*t >= Traj[0] + Traj[6+trajsegm*11])
                    {
                        trajsegm++;
                        *drivmode = (uint8_t)(Traj[14+trajsegm*11]+0.5);
                        if (*drivmode > 0)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
    if (*drivmode > 0)
    {
        vd[3] = sin(Traj[3]);
        vd[4] = cos(Traj[3]);
        vd[5] = x0[0] - Traj[1];
        vd[6] = x0[1] - Traj[2];
        vd[0] = +(vd[5]*vd[4]) + (vd[6]*vd[3]);
        vd[1] = -(vd[5]*vd[3]) + (vd[6]*vd[4]);
        if (trajsegm < 1)
        {
            vd[3] = vd[0];
            vd[4] = vd[1];
            vd[5] = Traj[7];
            vd[6] = Traj[8];
            vd[7] = (vd[5]*vd[5]) + (vd[6]*vd[6]);
            if (vd[7] <= 0)
            {
                vd[7] = 0;
            }
            else
            {
                vd[7] = ((vd[5]*vd[3])+(vd[6]*vd[4])) / vd[7];
                if      (vd[7] < 0)
                {
                    vd[7] = 0;
                }
                else if (vd[7] > 1)
                {
                    vd[7] = 1;
                }
            }
            vd[3] = vd[3] - (vd[7]*vd[5]);
            vd[4] = vd[4] - (vd[7]*vd[6]);
            vd[8] = (vd[3]*vd[3]) + (vd[4]*vd[4]);
            vd[9] = vd[7];
        }
        else
        {
            vi[1] = 6 + (trajsegm-1)*11;
            vd[3] = vd[0] - Traj[vi[1]+1];
            vd[4] = vd[1] - Traj[vi[1]+2];
            vd[5] = Traj[vi[1]+12] - Traj[vi[1]+1];
            vd[6] = Traj[vi[1]+13] - Traj[vi[1]+2];
            vd[7] = (vd[5]*vd[5]) + (vd[6]*vd[6]);
            if (vd[7] <= 0)
            {
                vd[7] = 0;
            }
            else
            {
                vd[7] = ((vd[5]*vd[3])+(vd[6]*vd[4])) / vd[7];
                if (vd[7] < 0)
                {
                    vd[7] = 0;
                }
                else if (vd[7] > 1)
                {
                    vd[7] = 1;
                }
            }
            vd[3] = vd[3] - (vd[7]*vd[5]);
            vd[4] = vd[4] - (vd[7]*vd[6]);
            vd[8] = (vd[3]*vd[3]) + (vd[4]*vd[4]);
            vd[9] = vd[7];
        }
        vi[0] = trajsegm;
        if (vi[3] == 0)
        {
            vi[2] = 0;
        }
        else
        {
            vi[2] = -vi[6];
        }
        while (vi[2] < 10)
        {
            if ((vi[7]==2) && ((vi[0]>vi[6]-2)||(vi[0]>2498)))
            {
                vi[0] = -1;
            }
            if ((vi[7]!=2) && ((vi[0]>(vi[6]-2))||(vi[0]>2498)))
            {
                break;
            }
            else
            {
                vi[0]++;
                vi[1] = 6 + vi[0]*11;
                if ((uint8_t)(Traj[vi[1]+8]+0.5) != (*drivmode))
                {
                    break;
                }
                else
                {
                    if (vi[0] < 1)
                    {
                        vd[3] = vd[0];
                        vd[4] = vd[1];
                        vd[5] = Traj[7];
                        vd[6] = Traj[8];
                    }
                    else
                    {
                        vd[3] = vd[0] - Traj[vi[1]-10];
                        vd[4] = vd[1] - Traj[vi[1]-9];
                        vd[5] = Traj[vi[1]+1] - Traj[vi[1]-10];
                        vd[6] = Traj[vi[1]+2] - Traj[vi[1]-9];
                    }
                    vd[7] = (vd[5]*vd[5]) + (vd[6]*vd[6]);
                    if (vd[7] <= 0)
                    {
                        vd[7] = 0;
                    }
                    else
                    {
                        vd[7] = ((vd[5]*vd[3])+(vd[6]*vd[4])) / vd[7];
                        if (vd[7] < 0)
                        {
                            vd[7] = 0;
                        }
                        else if (vd[7] > 1)
                        {
                            vd[7] = 1;
                        }
                    }
                    vd[3] = vd[3] - vd[7]*vd[5];
                    vd[4] = vd[4] - vd[7]*vd[6];
                    vd[6] = (vd[3]*vd[3]) + (vd[4]*vd[4]);
                    if (vd[6] < vd[8])
                    {
                        trajsegm = vi[0];
                        vd[8] = vd[6];
                        vd[9] = vd[7];
                        if (vi[2] > 0)
                        {
                            vi[2] = 0;
                        }
                    }
                    else
                    {
                        vi[2]++;
                    }
                }
            }
        }
        if (vd[9] > 0.9999)
        {
            trajsegm++;
            vd[9] = 0;
            if ((trajsegm>(vi[6]-1)) || (trajsegm>2499))
            {
                if (vi[7] < 2)
                {
                    *drivmode = 0;
                }
                else
                {
                    trajsegm = 0;
                    *drivmode = (uint8_t)(Traj[14]+0.5);
                }
            }
            else
            {
                *drivmode = (uint8_t)(Traj[14+trajsegm*11]+0.5);
            }
            if (*drivmode < 1)
            {
                while ((trajsegm<vi[6]) && (trajsegm<2500))
                {
                    if (*t >= Traj[0] + Traj[6+trajsegm*11])
                    {
                        trajsegm++;
                        *drivmode = (uint8_t)(Traj[14+trajsegm*11]+0.5);
                        if (*drivmode > 0)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        if ((x0[3]==0) && (vd[9]>0.8) )
        {
            if ((trajsegm<vi[6]-1) && (trajsegm<2499))
            {
                if (Traj[14+trajsegm*11] != Traj[25+trajsegm*11])
                {
                    trajsegm++;
                    *drivmode = (uint8_t)(Traj[14+trajsegm*11]+0.5);
                    vd[9] = 0;
                }
            }
            else if (((trajsegm==vi[6]-1) && (trajsegm<2500)) && (vi[7]==2))
            {
                if (Traj[14+trajsegm*11] != Traj[14])
                {
                    trajsegm = 0;
                    *drivmode = (uint8_t)(Traj[14]+0.5);
                    vd[9] = 0;
                }
            }
        }
    }
    if ((*drivmode>0) && ((trajsegm<vi[6])&&(trajsegm<2500)))
    {
        vi[0] = trajsegm;
        vi[1] = 6 + vi[0]*11;
        vd[0] = cos(Traj[3]);
        vd[1] = sin(Traj[3]);
        if (x0[3] < 0)
        {
            vd[5] = 0;
        }
        else
        {
            vd[5] = x0[3];
        }
        if (vi[0] < 1)
        {
            vd[8] = sqrt(Traj[7]*Traj[7] +
                         Traj[8]*Traj[8]);
        }
        else
        {
            vd[8] = sqrt((Traj[vi[1]+1]-Traj[vi[1]-10])*(Traj[vi[1]+1]-Traj[vi[1]-10]) +
                         (Traj[vi[1]+2]-Traj[vi[1]-9])*(Traj[vi[1]+2]-Traj[vi[1]-9]));
        }
        for (vi[2]=0; vi[2]<40; vi[2]++)
        {
            if (vi[7] > 0)
            {
                vd[4] = (Traj[vi[1]+4]-vd[5])/1.00000000000000006e-01;
                if      (vd[4] > (*amax))
                {
                    vd[4] = (*amax);
                }
                else if (vd[4] < (*amin))
                {
                    vd[4] = (*amin);
                }
            }
            else
            {
                if (vi[0] < 1)
                {
                    vd[7] = (*t) + vi[2]*1.00000000000000006e-01 - Traj[0] - (vd[9]*Traj[6]);
                }
                else
                {
                    vd[7] = (*t) + vi[2]*1.00000000000000006e-01 - Traj[0] - (vd[9]*Traj[6+vi[0]*11]) + ((vd[9]-1)*Traj[6+(vi[0]-1)*11]);
                }
                if (vd[7] >= 1.00000000000000000e+01)
                {
                    vd[6] = 1.02000000000000002e+00;
                }
                else
                {
                    vd[6] = 1.00000000000000000e+01 / (1.00000000000000000e+01 - vd[7]);
                    if      (vd[6] > 1.02000000000000002e+00)
                    {
                        vd[6] = 1.02000000000000002e+00;
                    }
                    else if (vd[6] < 9.79999999999999982e-01)
                    {
                        vd[6] = 9.79999999999999982e-01;
                    }
                }
                vd[7] = vd[7] * Traj[vi[1]+4];
                vd[6] = vd[5] - (vd[6]*Traj[vi[1]+4]);
                if      (vd[6] > 0)
                {
                    vd[3] = - vd[6] / (*amin);
                }
                else if (vd[6] < 0)
                {
                    vd[3] = - vd[6] / (*amax);
                }
                else
                {
                    vd[3] = 1.00000000000000006e-01;
                }
                vi[3] = (int64_t)(vd[3]/1.00000000000000006e-01 + 0.9999);
                vd[3] = vi[3] * 1.00000000000000006e-01;
                if (vd[3] < 0)
                {
                    vd[3] = 0;
                }
                vd[3] = 0.5 * vd[3] * vd[6];
                if      (vd[7] < vd[3])
                {
                    if (vd[6] >= -(*amin)*1.00000000000000006e-01)
                    {
                        vd[4] = (*amin);
                    }
                    else
                    {
                        vd[2] = - (vd[6]+1.00000000000000006e-01*(*amin)) / (*amax);
                        vi[4] = vd[2]/1.00000000000000006e-01 + 0.999999999999;
                        vd[2] = vi[4] * 1.00000000000000006e-01;
                        vd[2] = vd[6]*(0.5*vd[2]+1.00000000000000006e-01) + 5.00000000000000028e-02*(*amin)*(vd[2]+1.00000000000000006e-01);
                        if (vd[7] <= vd[2])
                        {
                            vd[4] = (*amin);
                        }
                        else
                        {
                            for (vi[5]=vi[3]-1; vi[5]<=vi[4]; vi[5]++)
                            {
                                vd[2] = vi[5] * 1.00000000000000006e-01;
                                vd[4] = (2*vd[7]-vd[6]*(vd[2]+2.00000000000000011e-01)) / 1.00000000000000006e-01 / (vd[2]+1.00000000000000006e-01);
                                if ((vd[4]<(*amax)) && (vd[4]>(*amin)))
                                {
                                    if ((vd[6]+vd[4]*1.00000000000000006e-01)/vd[2] > -(*amax))
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else if (vd[7] < vd[3])
                {
                    if (vd[6] <= -1.00000000000000006e-01*(*amax))
                    {
                        vd[4] = (*amax);
                    }
                    else
                    {
                        vd[2] = - (vd[6]+(*amax)*1.00000000000000006e-01) / (*amin);
                        vi[4] = vd[2]/1.00000000000000006e-01 + 0.999999999999;
                        vd[2] = vi[4] * 1.00000000000000006e-01;
                        vd[2] = vd[6]*(0.5*vd[2]+1.00000000000000006e-01) + 5.00000000000000028e-02*(*amax)*(vd[2]+1.00000000000000006e-01);
                        if (vd[7] >= vd[2])
                        {
                            vd[4] = (*amax);
                        }
                        else
                        {
                            for (vi[5]=vi[3]-1; vi[5]<=vi[4]; vi[5]++)
                            {
                                vd[2] = vi[5] * 1.00000000000000006e-01;
                                vd[4] = (2*vd[7]-vd[6]*(vd[2]+2.00000000000000011e-01)) / 1.00000000000000006e-01 / (vd[2]+1.00000000000000006e-01);
                                if ((vd[4]<(*amax)) && (vd[4]>(*amin)))
                                {
                                    if ((vd[6]+vd[4]*1.00000000000000006e-01)/vd[2] < -(*amin))
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (vi[3] < 1)
                    {
                        vd[4] = 0;
                    }
                    else
                    {
                        vd[4] = - vd[6] / vi[3] / 1.00000000000000006e-01;
                    }
                }
                if      (vd[4] > (*amax))
                {
                    vd[4] = (*amax);
                }
                else if (vd[4] < (*amin))
                {
                    vd[4] = (*amin);
                }
            }
            vd[5] = vd[5] + vd[4]*1.00000000000000006e-01;
            if (vd[5] < 0)
            {
                vd[5] = 0;
            }
            vd[4] = vd[5]*1.00000000000000006e-01;
            vd[9] = vd[9] + (vd[4]/vd[8]);
            if ((vd[9]>1) && ((vi[0]>vi[6]-2)||(vi[0]>2498)))
            {
                if (vi[7] < 2)
                {
                    vi[0]++;
                    vi[1] = vi[1] + 11;
                }
                else
                {
                    vi[0] = -1;
                    vi[1] = -5;
                }
            }
            while ((vd[9]>1) && ((vi[0]<vi[6]-1)&&(vi[0]<2499)))
            {
                vi[0]++;
                vi[1] = vi[1] + 11;
                if ((uint8_t)(Traj[vi[1]+8]+0.5) != (*drivmode))
                {
                    break;
                }
                else
                {
                    vd[4] = (vd[9]-1) * vd[8];
                    if (vi[0] < 1)
                    {
                         vd[8] = sqrt(Traj[7]*Traj[7] +
                                      Traj[8]*Traj[8]);
                    }
                    else
                    {
                        vd[8] = sqrt((Traj[vi[1]+1]-Traj[vi[1]-10])*(Traj[vi[1]+1]-Traj[vi[1]-10]) +
                                     (Traj[vi[1]+2]-Traj[vi[1]-9])*(Traj[vi[1]+2]-Traj[vi[1]-9]));
                    }
                    vd[9] = vd[4] / vd[8];
                    if ((vd[9]>1) && ((vi[0]>vi[6]-2)||(vi[0]>2498)))
                    {
                        if (vi[7] < 2)
                        {
                            vi[0]++;
                            vi[1] = vi[1] + 11;
                        }
                        else
                        {
                            vi[0] = -1;
                            vi[1] = -5;
                        }
                    }
                }
            }
            if ((vi[0]<vi[6]) && (vi[0]<2500))
            {
                if ((uint8_t)(Traj[vi[1]+8]+0.5) != (*drivmode))
                {
                    vi[0]--;
                    vi[1] = 6 + vi[0]*11;
                    while (vi[2]<40)
                    {
                        vi[3] = 9 * vi[2];
                        if (vi[0] < 1)
                        {
                            Ref[vi[3]  ] = Traj[1] + vd[0]*Traj[7] - vd[1]*Traj[8];
                            Ref[vi[3]+1] = Traj[2] + vd[1]*Traj[7] + vd[0]*Traj[8];
                            if (vi[2] < 1)
                            {
                                vd[10] = ((x0[2]-Traj[3]-Traj[9]) / 6.283185307179586) + 0.5;
                                if (vd[10] >= 0)
                                {
                                    vi[4] = (int64_t)(vd[10]);
                                }
                                else
                                {
                                    vi[4] = (int64_t)(vd[10]) - 1;
                                }
                                vd[10] = vi[4] * 6.283185307179586;
                            }
                            else
                            {
                                vd[10] = ((Ref[vi[3]-7]-Traj[3]-Traj[9]) / 6.283185307179586) + 0.5;
                                if (vd[10] >= 0)
                                {
                                    vi[4] = (int64_t)(vd[10]);
                                }
                                else
                                {
                                    vi[4] = (int64_t)(vd[10]) - 1;
                                }
                                vd[10] = vi[4] * 6.283185307179586;
                            }
                            Ref[vi[3]+2] = Traj[3] + Traj[9] + vd[10];
                            Ref[vi[3]+3] = 0;
                            Ref[vi[3]+4] = (*amin);
                            Ref[vi[3]+5] = Traj[12];
                            Ref[vi[3]+6] = Traj[13];
                            Ref[vi[3]+7] = Traj[15];
                            Ref[vi[3]+8] = Traj[16];
                        }
                        else
                        {
                            Ref[vi[3]  ] = Traj[1] + vd[0]*Traj[vi[1]+1] - vd[1]*Traj[vi[1]+2];
                            Ref[vi[3]+1] = Traj[2] + vd[1]*Traj[vi[1]+1] + vd[0]*Traj[vi[1]+2];
                            if (vi[2] < 1)
                            {
                                vd[10] = ((x0[2]-Traj[3]-Traj[vi[1]+3]) / 6.283185307179586) + 0.5;
                                if (vd[10] >= 0)
                                {
                                    vi[4] = (int64_t)(vd[10]);
                                }
                                else
                                {
                                    vi[4] = (int64_t)(vd[10]) - 1;
                                }
                                vd[10] = vi[4] * 6.283185307179586;
                            }
                            else
                            {
                                vd[10] = ((Ref[vi[3]-7]-Traj[3]-Traj[vi[1]+3]) / 6.283185307179586) + 0.5;
                                if (vd[10] >= 0)
                                {
                                    vi[4] = (int64_t)(vd[10]);
                                }
                                else
                                {
                                    vi[4] = (int64_t)(vd[10]) - 1;
                                }
                                vd[10] = vi[4] * 6.283185307179586;
                            }
                            Ref[vi[3]+2] = Traj[3] + Traj[vi[1]+3] + vd[10];
                            Ref[vi[3]+3] = 0;
                            Ref[vi[3]+4] = (*amin);
                            Ref[vi[3]+5] = Traj[vi[1]+6];
                            Ref[vi[3]+6] = Traj[vi[1]+7];
                            Ref[vi[3]+7] = Traj[vi[1]+9];
                            Ref[vi[3]+8] = Traj[vi[1]+10];
                        }
                        vi[2]++;
                    }
                }
                else
                {
                    vi[3] = 9 * vi[2];
                    if (vi[0] < 1)
                    {
                        vd[2] = vd[9] * Traj[7];
                        vd[3] = vd[9] * Traj[8];
                    }
                    else
                    {
                        vd[2] = vd[9]*Traj[vi[1]+1] + (1-vd[9])*Traj[vi[1]-10];
                        vd[3] = vd[9]*Traj[vi[1]+2] + (1-vd[9])*Traj[vi[1]-9];
                    }
                    Ref[vi[3]  ] = Traj[1] + vd[0]*vd[2] - vd[1]*vd[3];
                    Ref[vi[3]+1] = Traj[2] + vd[1]*vd[2] + vd[0]*vd[3];
                    if (vi[2] < 1)
                    {
                        vd[10] = ((x0[2]-Traj[3]-Traj[vi[1]+3]) / 6.283185307179586) + 0.5;
                        if (vd[10] >= 0)
                        {
                            vi[4] = (int64_t)(vd[10]);
                        }
                        else
                        {
                            vi[4] = (int64_t)(vd[10]) - 1;
                        }
                        vd[10] = vi[4] * 6.283185307179586;
                    }
                    else
                    {
                        vd[10] = ((Ref[vi[3]-7]-Traj[3]-Traj[vi[1]+3]) / 6.283185307179586) + 0.5;
                        if (vd[10] >= 0)
                        {
                            vi[4] = (int64_t)(vd[10]);
                        }
                        else
                        {
                            vi[4] = (int64_t)(vd[10]) - 1;
                        }
                        vd[10] = vi[4] * 6.283185307179586;
                    }
                    Ref[vi[3]+2] = Traj[3] + Traj[vi[1]+3] + vd[10];
                    Ref[vi[3]+3] = Traj[vi[1]+4];
                    if (Traj[vi[1]+5] < (*amin))
                    {
                        Ref[vi[3]+4] = (*amin);
                    }
                    else if (Traj[vi[1]+5] > (*amax))
                    {
                        Ref[vi[3]+4] = (*amax);
                    }
                    else
                    {
                        Ref[vi[3]+4] = Traj[vi[1]+5];
                    }
                    Ref[vi[3]+5] = Traj[vi[1]+6];
                    Ref[vi[3]+6] = Traj[vi[1]+7];
                    Ref[vi[3]+7] = Traj[vi[1]+9];
                    Ref[vi[3]+8] = Traj[vi[1]+10];
                }
            }
            else
            {
                while (vi[2]<40)
                {
                    if (2500 < vi[6])
                    {
                        vi[1] = 27495;
                    }
                    else
                    {
                        vi[1] = 6 + (vi[6]-1)*11;
                    }
                    vi[3] = 9 * vi[2];
                    Ref[vi[3]  ] = Traj[1] + vd[0]*Traj[vi[1]+1] - vd[1]*Traj[vi[1]+2];
                    Ref[vi[3]+1] = Traj[2] + vd[1]*Traj[vi[1]+1] + vd[0]*Traj[vi[1]+2];
                    if (vi[2] < 1)
                    {
                        vd[10] = ((x0[2]-Traj[3]-Traj[vi[1]+3]) / 6.283185307179586) + 0.5;
                        if (vd[10] >= 0)
                        {
                            vi[4] = (int64_t)(vd[10]);
                        }
                        else
                        {
                            vi[4] = (int64_t)(vd[10]) - 1;
                        }
                        vd[10] = vi[4] * 6.283185307179586;
                    }
                    else
                    {
                        vd[10] = ((Ref[vi[3]-7]-Traj[3]-Traj[vi[1]+3]) / 6.283185307179586) + 0.5;
                        if (vd[10] >= 0)
                        {
                            vi[4] = (int64_t)(vd[10]);
                        }
                        else
                        {
                            vi[4] = (int64_t)(vd[10]) - 1;
                        }
                        vd[10] = vi[4] * 6.283185307179586;
                    }
                    Ref[vi[3]+2] = Traj[3] + Traj[vi[1]+3] + vd[10];
                    Ref[vi[3]+3] = 0;
                    Ref[vi[3]+4] = (*amin);
                    Ref[vi[3]+5] = Traj[vi[1]+6];
                    Ref[vi[3]+6] = Traj[vi[1]+7];
                    Ref[vi[3]+7] = Traj[vi[1]+9];
                    Ref[vi[3]+8] = Traj[vi[1]+10];
                    vi[2]++;
                }
            }
        }
    }
    else if ((trajsegm<vi[6]) && (trajsegm<2500))
    {
        vi[1] = 6 + (trajsegm)*11;
        Ref[4] = Traj[vi[1]+5];
        if (Ref[4] > 0)
        {
            Ref[4] = 0;
        }
        vd[10] = Traj[0] + Traj[vi[1]] - (*t);
        if (vd[10] > 1.00000000000000006e-01)
        {
            Ref[5] = x0[4] + ((Traj[vi[1]+6] - x0[4]) * 1.00000000000000006e-01 / vd[10]);
        }
        else
        {
            Ref[5] = Traj[vi[1]+6];
        }
    }
    else if (vi[6] <= 2500)
    {
        vi[1] = 6 + vi[6]*11;
        Ref[4] = Traj[vi[1]+5];
        if (Ref[4] > 0)
        {
            Ref[4] = 0;
        }
        vd[10] = Traj[0] + Traj[vi[1]] - (*t);
        if (vd[10] > 1.00000000000000006e-01)
        {
            Ref[5] = x0[4] + ((Traj[vi[1]+6] - x0[4]) * 1.00000000000000006e-01 / vd[10]);
        }
        else
        {
            Ref[5] = Traj[vi[1]+6];
        }
    }
    else
    {
        Ref[4] = (*amin);
        Ref[5] = 0;
    }
}
static void gssim(const double *U, double *X, const int64_t *N, const uint8_t *drivmode, int64_t *vi, double *vd)
{
    if (X[3] < 0)
    {
    	 X[3] = 0;
    }
    if      (*drivmode == 1)
    {
        for (vi[0]=1; vi[0]<(*N)+1; vi[0]++)
        {
            vi[2] = vi[0]*2;
            vi[3] = vi[0]*5;
            X[vi[3]+0] = X[vi[3]-5];
            X[vi[3]+1] = X[vi[3]-4];
            X[vi[3]+2] = X[vi[3]-3];
            X[vi[3]+3] = X[vi[3]-2];
            X[vi[3]+4] = X[vi[3]-1];
            for (vi[1]=0; vi[1]<1; vi[1]++)
            {
                vd[0] = X[vi[3]+0];
                vd[1] = X[vi[3]+1];
                vd[2] = X[vi[3]+2];
                vd[3] = X[vi[3]+3];
                vd[4] = X[vi[3]+4];
                vd[5] = vd[3]*cos(vd[2]+atan((0.6113)*tan(vd[4])));
                vd[6] = vd[3]*sin(vd[2]+atan((0.6113)*tan(vd[4])));
                vd[7] = vd[3]/(2.843)*cos(atan((0.6113)*tan(vd[4])))*tan(vd[4]);
                vd[8] = U[vi[2]  ];
                vd[9] = U[vi[2]+1];

                X[vi[3]  ] = X[vi[3]  ] + (1.000000e-01) * vd[5];
                X[vi[3]+1] = X[vi[3]+1] + (1.000000e-01) * vd[6];
                X[vi[3]+2] = X[vi[3]+2] + (1.000000e-01) * vd[7];
                X[vi[3]+3] = X[vi[3]+3] + (1.000000e-01) * vd[8];
                if (X[vi[3]+3] < 0)
                {
                    X[vi[3]+3] = 0;
                }
                X[vi[3]+4] = X[vi[3]+4] + (1.000000e-01) * vd[9];

            }
        }
    }
    else if (*drivmode == 2)
    {
        for (vi[0]=1; vi[0]<(*N)+1; vi[0]++)
        {
            vi[2] = vi[0]*2;
            vi[3] = vi[0]*5;
            X[vi[3]+0] = X[vi[3]-5];
            X[vi[3]+1] = X[vi[3]-4];
            X[vi[3]+2] = X[vi[3]-3];
            X[vi[3]+3] = X[vi[3]-2];
            X[vi[3]+4] = X[vi[3]-1];
            for (vi[1]=0; vi[1]<1; vi[1]++)
            {
                vd[0] = X[vi[3]+0];
                vd[1] = X[vi[3]+1];
                vd[2] = X[vi[3]+2];
                vd[3] = - X[vi[3]+3];
                vd[4] = X[vi[3]+4];
                vd[5] = vd[3]*cos(vd[2]+atan((0.6113)*tan(vd[4])));
                vd[6] = vd[3]*sin(vd[2]+atan((0.6113)*tan(vd[4])));
                vd[7] = vd[3]/(2.843)*cos(atan((0.6113)*tan(vd[4])))*tan(vd[4]);
                vd[8] = U[vi[2]  ];
                vd[9] = U[vi[2]+1];

                X[vi[3]  ] = X[vi[3]  ] + (1.000000e-01) * vd[5];
                X[vi[3]+1] = X[vi[3]+1] + (1.000000e-01) * vd[6];
                X[vi[3]+2] = X[vi[3]+2] + (1.000000e-01) * vd[7];
                X[vi[3]+3] = X[vi[3]+3] + (1.000000e-01) * vd[8];
                if (X[vi[3]+3] < 0)
                {
                    X[vi[3]+3] = 0;
                }
                X[vi[3]+4] = X[vi[3]+4] + (1.000000e-01) * vd[9];

            }
        }
    }

}

 
 
static void gslin(double *U, double *X, double *A, double *B, const uint8_t *drivmode, int64_t *vi, double *vd)
{
    vi[0] = 40;
    gssim(U,X,&vi[0],drivmode,&vi[1],&vd[0]);
    vi[7] = 1;
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[3] = 2 * vi[0];
        vi[4] = 5 * (vi[0]+1);
        vi[5] = 10 * vi[0];
        vi[6] = 25 * vi[0];
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            vd[vi[1]+2]   = U[vi[3]+vi[1]+2];
        }
        for (vi[1]=0; vi[1]<5; vi[1]++)
        {
            vd[vi[1]+4] = X[vi[4]-5+vi[1]];
        }
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            vd[0] = vd[vi[1]+2];
            vd[vi[1]+2] = vd[vi[1]+2] + 1.00000000000000005e-04;
            gssim(&vd[0],&vd[4],&vi[7],drivmode,&vi[8],&vd[14]);
            for (vi[2]=0; vi[2]<5; vi[2]++)
            {
                B[vi[5]+(vi[2]*2)+vi[1]] = (vd[9+vi[2]] - X[vi[4]+vi[2]]) / 1.00000000000000005e-04;
            }
            vd[vi[1]+2] = vd[0];
        }
        for (vi[1]=0; vi[1]<5; vi[1]++)
        {
            vd[0] = vd[vi[1]+4];
            vd[vi[1]+4] = vd[vi[1]+4] + 1.00000000000000005e-04;
            gssim(&vd[0],&vd[4],&vi[7],drivmode,&vi[8],&vd[14]);
            for (vi[2]=0; vi[2]<5; vi[2]++)
            {
                A[vi[6]+(vi[2]*5)+vi[1]] = (vd[9+vi[2]] - X[vi[4]+vi[2]]) / 1.00000000000000005e-04;
            }
            vd[vi[1]+4] = vd[0];
        }
    }
}
 
static void gseval(const double *U, const double *X, const double *x, const double *alpha, const double *Ref, const uint8_t *drivmode, const double *R, const double *Q, const double *contolerance, const double *conpenalty, double *DJ, int64_t *vi, double *vd)
{
    for (vi[0]=0; vi[0]<2; vi[0]++)
    {
        vd[vi[0]] = U[vi[0]];
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[2] = (vi[0]+1) * 2;
        vi[3] = vi[0] * 7;
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            vd[vi[2]+vi[1]] = U[vi[2]+vi[1]] + (*alpha)*x[vi[3]+vi[1]];
        }
    }
    for (vi[0]=0; vi[0]<5; vi[0]++)
    {
        vd[82+vi[0]] = X[vi[0]];
    }
    vi[0] = 40;
    gssim(&vd[0],&vd[82],&vi[0],drivmode,&vi[1],&vd[(41*7)]);
    DJ[0] = 0;
    DJ[1] = 0;
    DJ[2] = 0;
    DJ[3] = 0;
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[1] = (vi[0]*9) + 2;
        vi[2] = ((vi[0]+1)*5) + 2;
        vi[3] = 82 + vi[2];
        vd[(41*7)] = (vd[vi[3]]-Ref[vi[1]])*(vd[vi[3]]-Ref[vi[1]]) -
                                       ( X[vi[2]]-Ref[vi[1]])*( X[vi[2]]-Ref[vi[1]]);
        vd[(41*7)] = 0.5 * Q[2] * vd[(41*7)];
        vd[288] = vd[(41*7)] + DJ[0];
        if (fabs(vd[(41*7)]) > fabs(DJ[0]))
        {
            vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
        }
        else
        {
            vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
        }
        DJ[0] = vd[288];
        vd[288] = vd[289] + DJ[1];
        if (fabs(DJ[1]) > fabs(vd[289]))
        {
            vd[289] = (DJ[1] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[1];
        }
        DJ[1] = vd[288];
        vd[288] = vd[289] + DJ[2];
        if (fabs(DJ[2]) > fabs(vd[289]))
        {
            vd[289] = (DJ[2] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[2];
        }
        DJ[2] = vd[288];
        DJ[3] = DJ[3] + vd[289];
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[1] = (vi[0]*9) + 3;
        vi[2] = ((vi[0]+1)*5) + 3;
        vi[3] = 82 + vi[2];
        vd[(41*7)] = (vd[vi[3]]-Ref[vi[1]])*(vd[vi[3]]-Ref[vi[1]]) -
                                       ( X[vi[2]]-Ref[vi[1]])*( X[vi[2]]-Ref[vi[1]]);
        vd[(41*7)] = 0.5 * Q[3] * vd[(41*7)];
        vd[288] = vd[(41*7)] + DJ[0];
        if (fabs(vd[(41*7)]) > fabs(DJ[0]))
        {
            vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
        }
        else
        {
            vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
        }
        DJ[0] = vd[288];
        vd[288] = vd[289] + DJ[1];
        if (fabs(DJ[1]) > fabs(vd[289]))
        {
            vd[289] = (DJ[1] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[1];
        }
        DJ[1] = vd[288];
        vd[288] = vd[289] + DJ[2];
        if (fabs(DJ[2]) > fabs(vd[289]))
        {
            vd[289] = (DJ[2] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[2];
        }
        DJ[2] = vd[288];
        DJ[3] = DJ[3] + vd[289];
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[1] = (vi[0]*9) + 5;
        vi[2] = ((vi[0]+1)*5) + 4;
        vi[3] = 82 + vi[2];
        vd[(41*7)] = (vd[vi[3]]-Ref[vi[1]])*(vd[vi[3]]-Ref[vi[1]]) -
                                       ( X[vi[2]]-Ref[vi[1]])*( X[vi[2]]-Ref[vi[1]]);
        vd[(41*7)] = 0.5 * Q[4] * vd[(41*7)];
        vd[288] = vd[(41*7)] + DJ[0];
        if (fabs(vd[(41*7)]) > fabs(DJ[0]))
        {
            vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
        }
        else
        {
            vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
        }
        DJ[0] = vd[288];
        vd[288] = vd[289] + DJ[1];
        if (fabs(DJ[1]) > fabs(vd[289]))
        {
            vd[289] = (DJ[1] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[1];
        }
        DJ[1] = vd[288];
        vd[288] = vd[289] + DJ[2];
        if (fabs(DJ[2]) > fabs(vd[289]))
        {
            vd[289] = (DJ[2] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[2];
        }
        DJ[2] = vd[288];
        DJ[3] = DJ[3] + vd[289];
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[1] = (vi[0]*9) + 4;
        vi[2] = ((vi[0]+1)*2);
        vd[(41*7)] = (vd[vi[2]]-Ref[vi[1]])*(vd[vi[2]]-Ref[vi[1]]) -
                                       ( U[vi[2]]-Ref[vi[1]])*( U[vi[2]]-Ref[vi[1]]);
        vd[(41*7)] = 0.5 * R[0] * vd[(41*7)];
        vd[288] = vd[(41*7)] + DJ[0];
        if (fabs(vd[(41*7)]) > fabs(DJ[0]))
        {
            vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
        }
        else
        {
            vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
        }
        DJ[0] = vd[288];
        vd[288] = vd[289] + DJ[1];
        if (fabs(DJ[1]) > fabs(vd[289]))
        {
            vd[289] = (DJ[1] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[1];
        }
        DJ[1] = vd[288];
        vd[288] = vd[289] + DJ[2];
        if (fabs(DJ[2]) > fabs(vd[289]))
        {
            vd[289] = (DJ[2] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[2];
        }
        DJ[2] = vd[288];
        DJ[3] = DJ[3] + vd[289];
    }
    for (vi[1]=1; vi[1]<2; vi[1]++)
    {
        for (vi[0]=0; vi[0]<40; vi[0]++)
        {
            vi[2] = ((vi[0]+1)*2) + vi[1];
            vd[(41*7)] = (vd[vi[2]]*vd[vi[2]]) -
                                           ( U[vi[2]]* U[vi[2]]);
            vd[(41*7)] = 0.5 * R[vi[1]] * vd[(41*7)];
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[1] = (vi[0]*9);
        vi[2] = ((vi[0]+1)*5);
        vi[3] = 82 + vi[2];
        vd[(41*7)] = sin(Ref[vi[1]+2]);
        vd[288] = cos(Ref[vi[1]+2]);
        vd[290] = X[vi[2]  ] - Ref[vi[1]  ];
        vd[291] = X[vi[2]+1] - Ref[vi[1]+1];
        vd[292] = (vd[288]*vd[291]) -
                                       (vd[(41*7)]*vd[290]);
        vd[293] = (vd[288]*vd[290]) +
                                       (vd[(41*7)]*vd[291]);
        vd[294] = vd[vi[3]  ] - Ref[vi[1]  ];
        vd[295] = vd[vi[3]+1] - Ref[vi[1]+1];
        vd[290] = (vd[288]*vd[295]) -
                                       (vd[(41*7)]*vd[294]);
        vd[291] = (vd[288]*vd[294]) +
                                       (vd[(41*7)]*vd[295]);
        vd[(41*7)] = (vd[290]*vd[290]) -
                                       (vd[292]*vd[292]);
        vd[(41*7)]  = 0.5 * Q[0] * vd[(41*7)];
        vd[288] = vd[(41*7)] + DJ[0];
        if (fabs(vd[(41*7)]) > fabs(DJ[0]))
        {
            vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
        }
        else
        {
            vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
        }
        DJ[0] = vd[288];
        vd[288] = vd[289] + DJ[1];
        if (fabs(DJ[1]) > fabs(vd[289]))
        {
            vd[289] = (DJ[1] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[1];
        }
        DJ[1] = vd[288];
        vd[288] = vd[289] + DJ[2];
        if (fabs(DJ[2]) > fabs(vd[289]))
        {
            vd[289] = (DJ[2] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[2];
        }
        DJ[2] = vd[288];
        DJ[3] = DJ[3] + vd[289];
        vd[(41*7)] = (vd[291]*vd[291]) -
                                       (vd[293]*vd[293]);
        vd[(41*7)]  = 0.5 * Q[1] * vd[(41*7)];
        vd[288] = vd[(41*7)] + DJ[0];
        if (fabs(vd[(41*7)]) > fabs(DJ[0]))
        {
            vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
        }
        else
        {
            vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
        }
        DJ[0] = vd[288];
        vd[288] = vd[289] + DJ[1];
        if (fabs(DJ[1]) > fabs(vd[289]))
        {
            vd[289] = (DJ[1] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[1];
        }
        DJ[1] = vd[288];
        vd[288] = vd[289] + DJ[2];
        if (fabs(DJ[2]) > fabs(vd[289]))
        {
            vd[289] = (DJ[2] - vd[288]) + vd[289];
        }
        else
        {
            vd[289] = (vd[289] - vd[288]) + DJ[2];
        }
        DJ[2] = vd[288];
        DJ[3] = DJ[3] + vd[289];
        if      (+vd[290] > Ref[vi[1]+7] + (*contolerance))
        {
            vd[(41*7)]  = (*conpenalty) * (+vd[290]-Ref[vi[1]+7]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        else if (+vd[290] > Ref[vi[1]+7])
        {
            vd[293] = + vd[290] - Ref[vi[1]+7];
            vd[294] = vd[293] * vd[293];
            vd[295] = vd[293] * vd[294];
            vd[(41*7)]  = ((-(*conpenalty)/(*contolerance)/(*contolerance))*vd[295]) + (2*(*conpenalty)/(*contolerance)*vd[294]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        if      (-vd[290] > Ref[vi[1]+8] + (*contolerance))
        {
            vd[(41*7)]  = (*conpenalty) * (-vd[290]-Ref[vi[1]+8]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        else if (-vd[290] > Ref[vi[1]+8])
        {
            vd[293] = - vd[290] - Ref[vi[1]+8];
            vd[294] = vd[293] * vd[293];
            vd[295] = vd[293] * vd[294];
            vd[(41*7)]  = (-(*conpenalty)/(*contolerance)/(*contolerance)*vd[295]) + (2*(*conpenalty)/(*contolerance)*vd[294]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        if      (+vd[292] > Ref[vi[1]+7] + (*contolerance))
        {
            vd[(41*7)]  = (*conpenalty) * (-vd[292]+Ref[vi[1]+7]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        else if (+vd[292] > Ref[vi[1]+7])
        {
            vd[293] = + vd[292] - Ref[vi[1]+7];
            vd[294] = vd[293] * vd[293];
            vd[295] = vd[293] * vd[294];
            vd[(41*7)] = (+(*conpenalty)/(*contolerance)/(*contolerance)*vd[295]) - (2*(*conpenalty)/(*contolerance)*vd[294]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        if      (-vd[292] > Ref[vi[1]+8] + (*contolerance))
        {
            vd[(41*7)]  = (*conpenalty) * (+vd[292]+Ref[vi[1]+8]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
        else if (-vd[292] > Ref[vi[1]+8])
        {
            vd[293] = - vd[292] - Ref[vi[1]+8];
            vd[294] = vd[293] * vd[293];
            vd[295] = vd[293] * vd[294];
            vd[(41*7)]  = (+(*conpenalty)/(*contolerance)/(*contolerance)*vd[295]) - (2*(*conpenalty)/(*contolerance)*vd[294]);
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
    }
    for (vi[1]=5; vi[1]<5; vi[1]++)
    {
        for (vi[0]=0; vi[0]<40; vi[0]++)
        {
            vi[2] = ((vi[0]+1)*5) + vi[1];
            vi[3] = 82 + vi[2];
            vd[(41*7)] = (vd[vi[3]]*vd[vi[3]]) -
                                           ( X[vi[2]]* X[vi[2]]);
            vd[(41*7)] = 0.5 * Q[vi[1]] * vd[(41*7)];
            vd[288] = vd[(41*7)] + DJ[0];
            if (fabs(vd[(41*7)]) > fabs(DJ[0]))
            {
                vd[289] = (vd[(41*7)]-vd[288]) + DJ[0];
            }
            else
            {
                vd[289] = (DJ[0]-vd[288]) + vd[(41*7)];
            }
            DJ[0] = vd[288];
            vd[288] = vd[289] + DJ[1];
            if (fabs(DJ[1]) > fabs(vd[289]))
            {
                vd[289] = (DJ[1] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[1];
            }
            DJ[1] = vd[288];
            vd[288] = vd[289] + DJ[2];
            if (fabs(DJ[2]) > fabs(vd[289]))
            {
                vd[289] = (DJ[2] - vd[288]) + vd[289];
            }
            else
            {
                vd[289] = (vd[289] - vd[288]) + DJ[2];
            }
            DJ[2] = vd[288];
            DJ[3] = DJ[3] + vd[289];
        }
    }
}
 
static void gsactset(double *U, int8_t *Uas, int8_t *dUas, const double *UconMod, int64_t *vi, double *vd)
{
    for (vi[1]=0; vi[1]<2; vi[1]++)
    {
        vi[2] =   2 + vi[1];
        if      (UconMod[         vi[1]] < 0)
        {
            vd[         vi[1]] = (1-macheps)*UconMod[         vi[1]];
        }
        else if (UconMod[         vi[1]] > 0)
        {
            vd[         vi[1]] = (1+macheps)*UconMod[         vi[1]];
        }
        else
        {
            vd[         vi[1]] = macheps;
        }
        if      (UconMod[  2+vi[1]] > 0)
        {
            vd[  2+vi[1]] = (1-macheps)*UconMod[  2+vi[1]];
        }
        else if (UconMod[  2+vi[1]] < 0)
        {
            vd[  2+vi[1]] = (1+macheps)*UconMod[  2+vi[1]];
        }
        else
        {
            vd[  2+vi[1]] = (-1)*macheps;
        }
        if      (UconMod[4+vi[1]] < 0)
        {
            vd[4+vi[1]] = (1-macheps)*UconMod[4+vi[1]];
        }
        else if (UconMod[4+vi[1]] > 0)
        {
            vd[4+vi[1]] = (1+macheps)*UconMod[4+vi[1]];
        }
        else
        {
            vd[4+vi[1]] = macheps;
        }
        if      (UconMod[6+vi[1]] > 0)
        {
            vd[6+vi[1]] = (1-macheps)*UconMod[6+vi[1]];
        }
        else if (UconMod[6+vi[1]] < 0)
        {
            vd[6+vi[1]] = (1+macheps)*UconMod[6+vi[1]];
        }
        else
        {
            vd[6+vi[1]] = (-1)*macheps;
        }
        Uas[vi[1]] = 0;
        if (U[vi[2]] <= vd[       vi[1]])
        {
            U[vi[2]] = UconMod[       vi[1]];
            Uas[vi[1]] = -6;
        }
        if (U[vi[2]] >= vd[2+vi[1]])
        {
            U[vi[2]] = UconMod[2+vi[1]];
            Uas[vi[1]] = +6;
        }
        if (U[vi[2]] <= (U[vi[1]] + vd[4+vi[1]]))
        {
            U[vi[2]] = U[vi[1]] + UconMod[4+vi[1]];
            Uas[vi[1]] = -6;
        }
        if (U[vi[2]] >= (U[vi[1]] + vd[6+vi[1]]))
        {
            U[vi[2]] = U[vi[1]] + UconMod[6+vi[1]];
            Uas[vi[1]] = +6;
        }
        for (vi[0]=1; vi[0]<40; vi[0]++)
        {
            vi[2] = (vi[0]-1)*2 + vi[1];
            vi[3] =     vi[0]*2 + vi[1];
            vi[4] = (vi[0]+1)*2 + vi[1];
            if      (U[vi[4]] <= UconMod[       vi[1]])
            {
                U[vi[4]] = UconMod[       vi[1]];
            }
            else if (U[vi[4]] >= UconMod[2+vi[1]])
            {
                U[vi[4]] = UconMod[2+vi[1]];
            }
            dUas[vi[2]] = 0;
            if      (U[vi[4]] <= (U[vi[3]] + vd[4+vi[1]]))
            {
                U[vi[4]] = U[vi[3]] + UconMod[4+vi[1]];
                dUas[vi[2]] = -2;
            }
            else if (U[vi[4]] >= (U[vi[3]] + vd[6+vi[1]]))
            {
                U[vi[4]] = U[vi[3]] + UconMod[6+vi[1]];
                dUas[vi[2]] = +2;
            }
        }
        for (vi[0]=1; vi[0]<40; vi[0]++)
        {
            vi[2] = (vi[0]-1)*2+vi[1];
            vi[3] =     vi[0]*2+vi[1];
            vi[4] = (vi[0]+1)*2+vi[1];
            Uas[vi[3]] = 0;
            if      (U[vi[4]] <= vd[     vi[1]])
            {
                if ((dUas[vi[2]]<-1) || (dUas[vi[2]]>+1))
                {
                    Uas[vi[3]] = -2;
                    if (vi[2] >= 2)
                    {
                        if ((dUas[vi[2]-2]<-1) || (dUas[vi[2]-2]>+1))
                        {
                            vi[5] = 2;
                        }
                        else
                        {
                            vi[5] = 0;
                        }
                    }
                    else
                    {
                        vi[5] = 0;
                    }
                    while (vi[5] > 1)
                    {
                        vi[2] = vi[2] - 2;
                        if (vi[2] >= 2)
                        {
                            if ((dUas[vi[2]-2]>-1) && (dUas[vi[2]-2]<+1))
                            {
                                vi[5] = 0;
                            }
                        }
                        else
                        {
                            vi[5] = 0;
                        }
                    }
                    Uas[vi[2]] = -4;
                }
                else
                {
                    Uas[vi[3]] = -6;
                }
            }
            else if (U[vi[4]] >= vd[2+vi[1]])
            {
                if ((dUas[vi[2]]<-1) || (dUas[vi[2]]>+1))
                {
                    Uas[vi[3]] = +2;
                    if (vi[2] >= 2)
                    {
                        if ((dUas[vi[2]-2]<-1) || (dUas[vi[2]-2]>+1))
                        {
                            vi[5] = 2;
                        }
                        else
                        {
                            vi[5] = 0;
                        }
                    }
                    else
                    {
                        vi[5] = 0;
                    }
                    while (vi[5] > 1)
                    {
                        vi[2] = vi[2] - 2;
                        if (vi[2] >= 2)
                        {
                            if ((dUas[vi[2]-2]>-1) && (dUas[vi[2]-2]<+1))
                            {
                                vi[5] = 0;
                            }
                        }
                        else
                        {
                            vi[5] = 0;
                        }
                    }
                    Uas[vi[2]] = +4;
                }
                else
                {
                    Uas[vi[3]] = +6;
                }
            }
        }
    }
}
 
static void gseqcon(const int8_t *Uas, const int8_t *dUas, int8_t *E, int8_t *F, uint8_t *p, int16_t *efp, int64_t *vi)
{
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        p[vi[0]] = 0;
    }
    for (vi[1]=0; vi[1]<2; vi[1]++)
    {
        if      (Uas[vi[1]] < -3)
        {
            for (vi[2]=0; vi[2]<2; vi[2]++)
            {
                E[p[0]*2+vi[2]] = 0;
            }
            E[p[0]*2+vi[1]] = -1;
            efp[p[0]] = 1 + vi[1];
            p[0]++;
        }
        else if (Uas[vi[1]] > +3)
        {
            for (vi[2]=0; vi[2]<2; vi[2]++)
            {
                E[p[0]*2+vi[2]] = 0;
            }
            E[p[0]*2+vi[1]] = +1;
            efp[p[0]] = 1 + vi[1];
            p[0]++;
        }
    }
    for (vi[0]=1; vi[0]<40; vi[0]++)
    {
        vi[3] = (vi[0]-1) * 2;
        vi[4] =     vi[0] * 2;
        vi[5] = (vi[0]-1) * 4;
        vi[6] =     vi[0] * 4;
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            if      (Uas[vi[4]+vi[1]] < -3)
            {
                for (vi[2]=0; vi[2]<2; vi[2]++)
                {
                    F[vi[5]+p[vi[0]]*2+vi[2]] = 0;
                    E[vi[6]+p[vi[0]]*2+vi[2]] = 0;
                }
                E[vi[6]+p[vi[0]]*2+vi[1]] = -1;
                efp[vi[4]+p[vi[0]]] = 1 + vi[4] + vi[1];
                p[vi[0]]++;
            }
            else if (Uas[vi[4]+vi[1]] > +3)
            {
                for (vi[2]=0; vi[2]<2; vi[2]++)
                {
                    F[vi[5]+p[vi[0]]*2+vi[2]] = 0;
                    E[vi[6]+p[vi[0]]*2+vi[2]] = 0;
                }
                E[vi[6]+p[vi[0]]*2+vi[1]] = +1;
                efp[vi[4]+p[vi[0]]] = 1 + vi[4] + vi[1];
                p[vi[0]]++;
            }
            else if (dUas[vi[3]+vi[1]] < -1)
            {
                for (vi[2]=0; vi[2]<2; vi[2]++)
                {
                    E[vi[6]+p[vi[0]]*2+vi[2]] = 0;
                    F[vi[5]+p[vi[0]]*2+vi[2]] = 0;
                }
                E[vi[6]+p[vi[0]]*2+vi[1]] = -1;
                F[vi[5]+p[vi[0]]*2+vi[1]] = +1;
                efp[vi[4]+p[vi[0]]] = -1 - (vi[3] + vi[1]);
                p[vi[0]]++;
            }
            else if (dUas[vi[3]+vi[1]] > +1)
            {
                for (vi[2]=0; vi[2]<2; vi[2]++) {
                    E[vi[6]+p[vi[0]]*2+vi[2]] = 0;
                    F[vi[5]+p[vi[0]]*2+vi[2]] = 0;
                }
                E[vi[6]+p[vi[0]]*2+vi[1]] = +1;
                F[vi[5]+p[vi[0]]*2+vi[1]] = -1;
                efp[vi[4]+p[vi[0]]] = -1 - (vi[3] + vi[1]);
                p[vi[0]]++;
            }
        }
    }
}
 
static void gscostf(const double *U, const double *X, const double *Ref, const double *R, const double *Q, const double *contolerance, const double *conpenalty, double *fx, int64_t *vi, double *vd)
{
    for (vi[0]=0; vi[0]<40; vi[0]++) {
        vi[2] = vi[0] * 9;
        vi[3] = (vi[0]+1) * 2;
        vi[4] = vi[0] * 7;
        fx[vi[4]  ] = R[0] * (U[vi[3]  ]-Ref[vi[2]+4]);
        fx[vi[4]+1] = R[1] * (U[vi[3]+1]           );
        for (vi[1]=2; vi[1]<2; vi[1]++)
        {
            fx[vi[4]+vi[1]] = R[vi[1]] * U[vi[3]+vi[1]];
        }
        vd[0] = sin(Ref[vi[2]+2]);
        vd[1] = cos(Ref[vi[2]+2]);
        vd[2] = vd[1] * vd[0];
        vi[3] = (vi[0]+1) * 5;
        vi[4] = vi[4] + 2;
        fx[vi[4]  ] = (((vd[0]*vd[0]*Q[0])+(vd[1]*vd[1]*Q[1]))*(X[vi[3]  ]-Ref[vi[2]  ])) + ((Q[1]-Q[0])*vd[2]*(X[vi[3]+1]-Ref[vi[2]+1]));
        fx[vi[4]+1] = (((vd[0]*vd[0]*Q[1])+(vd[1]*vd[1]*Q[0]))*(X[vi[3]+1]-Ref[vi[2]+1])) + ((Q[1]-Q[0])*vd[2]*(X[vi[3]  ]-Ref[vi[2]  ]));
        fx[vi[4]+2] = Q[2] * (X[vi[3]+2]-Ref[vi[2]+2]);
        fx[vi[4]+3] = Q[3] * (X[vi[3]+3]-Ref[vi[2]+3]);
        fx[vi[4]+4] = Q[4] * (X[vi[3]+4]-Ref[vi[2]+5]);
        for (vi[1]=5; vi[1]<5; vi[1]++)
        {
            fx[vi[4]+vi[1]] = Q[vi[1]] * X[vi[3]+vi[1]];
        }
        vd[2] = (vd[0]*(Ref[vi[2]  ]-X[vi[3]  ])) + (vd[1]*(X[vi[3]+1]-Ref[vi[2]+1]));
        if      (+vd[2] > Ref[vi[2]+7] + (*contolerance))
        {
            fx[vi[4]  ] = fx[vi[4]  ] - (vd[0]*(*conpenalty));
            fx[vi[4]+1] = fx[vi[4]+1] + (vd[1]*(*conpenalty));
        }
        else if (+vd[2] > Ref[vi[2]+7])
        {
            vd[3] = + vd[2] - Ref[vi[2]+7];
            fx[vi[4]  ] = fx[vi[4]  ] - (vd[0]*vd[3]*(*conpenalty)/(*contolerance)*(4-(3/(*contolerance)*vd[3])));
            fx[vi[4]+1] = fx[vi[4]+1] + (vd[1]*vd[3]*(*conpenalty)/(*contolerance)*(4-(3/(*contolerance)*vd[3])));
        }
        if      (-vd[2] > Ref[vi[2]+8] + (*contolerance))
        {
            fx[vi[4]  ] = fx[vi[4]  ] + (vd[0]*(*conpenalty));
            fx[vi[4]+1] = fx[vi[4]+1] - (vd[1]*(*conpenalty));
        }
        else if (-vd[2] > Ref[vi[2]+8])
        {
            vd[3] = - vd[2] - Ref[vi[2]+8];
            fx[vi[4]  ] = fx[vi[4]  ] + (vd[0]*vd[3]*(*conpenalty)/(*contolerance)*(4-(3/(*contolerance)*vd[3])));
            fx[vi[4]+1] = fx[vi[4]+1] - (vd[1]*vd[3]*(*conpenalty)/(*contolerance)*(4-(3/(*contolerance)*vd[3])));
        }
    }
}
 
static void gskkt(const double *LR, const double *LQ, const double *Ref, const double *fx, const double *A, const double *B, const int8_t *E, const int8_t *F, const uint8_t *p, double *ghf, double *GHG, double *L, double *lambda, double *x, int64_t *vi, double *vd)
{
    vd[0] = Ref[2];
    vd[1] = sin(vd[0]);
    vd[2] = cos(vd[0]);
    vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
    vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
    vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        if (vd[0] != Ref[2+(vi[0]*9)])
        {
            vd[0] = Ref[2+(vi[0]*9)];
            vd[1] = sin(vd[0]);
            vd[2] = cos(vd[0]);
            vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
            vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
            vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
        }
        vi[2] = vi[0] * 7;
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            lambda[vi[2]+vi[1]] = - fx[vi[2]+vi[1]] / LR[vi[1]];
        }
        vi[2] = vi[0]*7 + 2;
        lambda[vi[2]  ] = - (fx[vi[2]]*vd[3]) - (fx[vi[2]+1]*vd[5]);
        lambda[vi[2]+1] = - (fx[vi[2]]*vd[5]) - (fx[vi[2]+1]*vd[4]);
        for (vi[1]=2; vi[1]<5; vi[1]++)
        {
            lambda[vi[2]+vi[1]] = - fx[vi[2]+vi[1]] / LQ[vi[1]];
        }
    }
    for (vi[2]=0; vi[2]<p[0]; vi[2]++)
    {
        vi[4] = vi[2];
        ghf[vi[4]] = 0;
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            ghf[vi[4]] = ghf[vi[4]] + (E[vi[2]*2+vi[1]]*lambda[vi[1]]);
        }
    }
    for (vi[2]=0; vi[2]<5; vi[2]++)
    {
        vi[4] = p[0] + vi[2];
        ghf[vi[4]] = B[vi[2]*2] * lambda[0];
        for (vi[1]=1; vi[1]<2; vi[1]++)
        {
            ghf[vi[4]] = ghf[vi[4]] + (B[vi[2]*2+vi[1]]*lambda[vi[1]]);
        }
        ghf[vi[4]] = ghf[vi[4]] - lambda[2+vi[2]];
    }
    for (vi[0]=1; vi[0]<40; vi[0]++)
    {
        vi[5] = (vi[0]-1) * 4;
        vi[6] = vi[0] * 4;
        vi[7] = (vi[0]-1) * 7;
        vi[8] = vi[0] * 7;
        vi[11] = vi[0] * 25;
        vi[12] = vi[0] * 10;
        for (vi[2]=0; vi[2]<p[vi[0]]; vi[2]++)
        {
            vi[4] = vi[8] + vi[2];
            ghf[vi[4]] = 0;
            for (vi[1]=0; vi[1]<2; vi[1]++)
            {
                ghf[vi[4]] = ghf[vi[4]] + (F[vi[5]+vi[2]*2+vi[1]]*lambda[vi[7]+vi[1]]);
            }
            for (vi[1]=0; vi[1]<2; vi[1]++)
            {
                ghf[vi[4]] = ghf[vi[4]] + (E[vi[6]+vi[2]*2+vi[1]]*lambda[vi[8]+vi[1]]);
            }
        }
        for (vi[2]=0; vi[2]<5; vi[2]++)
        {
            vi[4] = vi[8] + p[vi[0]] + vi[2];
            ghf[vi[4]] = - lambda[vi[8]+2+vi[2]];
            for (vi[1]=0; vi[1]<5; vi[1]++)
            {
                ghf[vi[4]] = ghf[vi[4]] + (A[vi[11]+vi[2]*5+vi[1]]*lambda[vi[7]+2+vi[1]]);
            }
            for (vi[1]=0; vi[1]<2; vi[1]++)
            {
                ghf[vi[4]] = ghf[vi[4]] + (B[vi[12]+vi[2]*2+vi[1]]*lambda[vi[8]+vi[1]]);
            }
        }
    }
    vd[0] = Ref[2];
    vd[1] = sin(vd[0]);
    vd[2] = cos(vd[0]);
    vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
    vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
    vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
    for (vi[2]=0; vi[2]<p[0]; vi[2]++)
    {
        vi[4] = (vi[2]*(vi[2]+1)) / 2;
        for (vi[1]=0; vi[1]<vi[2]+1; vi[1]++)
        {
            GHG[vi[4]+vi[1]] = E[vi[2]*2]*E[vi[1]*2]/LR[0];
            for (vi[3]=1; vi[3]<2; vi[3]++)
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (E[vi[2]*2+vi[3]]*E[vi[1]*2+vi[3]]/LR[vi[3]]);
            }
        }
    }
    for (vi[2]=0; vi[2]<5; vi[2]++)
    {
        vi[4] = ((p[0]+vi[2])*(p[0]+vi[2]+1)) / 2;
        for (vi[1]=0; vi[1]<p[0]; vi[1]++)
        {
            GHG[vi[4]+vi[1]] = B[vi[2]*2] * E[vi[1]*2] / LR[0];
            for (vi[3]=1; vi[3]<2; vi[3]++)
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (B[vi[2]*2+vi[3]]*E[vi[1]*2+vi[3]]/LR[vi[3]]);
            }
        }
    }
    for (vi[2]=0; vi[2]<5; vi[2]++)
    {
        vi[4] = (((p[0]+vi[2])*(p[0]+vi[2]+1))/2) + p[0];
        for (vi[1]=0; vi[1]<vi[2]+1; vi[1]++)
        {
            GHG[vi[4]+vi[1]] = B[vi[2]*2] * B[vi[1]*2] / LR[0];
            for (vi[3]=1; vi[3]<2; vi[3]++)
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (B[vi[2]*2+vi[3]]*B[vi[1]*2+vi[3]]/LR[vi[3]]);
            }
            if      ((vi[2]==0) && (vi[1]==0))
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + vd[3];
            }
            else if ((vi[2]==1) && (vi[1]==0))
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + vd[5];
            }
            else if ((vi[2]==1) && (vi[1]==1))
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + vd[4];
            }
            else if ((vi[2]>1) && (vi[2]==vi[1]))
            {
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (1/LQ[vi[2]]);
            }
        }
    }
    for (vi[0]=1; vi[0]<40; vi[0]++)
    {
        vi[5] = (vi[0]-1) * 4;
        vi[6] = vi[0] * 4;
        vi[8] = vi[0] * 10;
        vi[9] = vi[0] * 25;
        for (vi[2]=0; vi[2]<p[vi[0]]; vi[2]++)
        {
            vi[4] = (vi[0]*28) + ((vi[2]*(vi[2]+1))/2);
            for (vi[1]=0; vi[1]<vi[2]+1; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = E[vi[6]+vi[2]*2] * E[vi[6]+vi[1]*2] / LR[0];
                for (vi[3]=1; vi[3]<2; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (E[vi[6]+vi[2]*2+vi[3]]*E[vi[6]+vi[1]*2+vi[3]]/LR[vi[3]]);
                }
                for (vi[3]=0; vi[3]<2; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (F[vi[5]+vi[2]*2+vi[3]]*F[vi[5]+vi[1]*2+vi[3]]/LR[vi[3]]);
                }
            }
        }
        for (vi[2]=0; vi[2]<5; vi[2]++)
        {
            vi[4] = (vi[0]*28) + (((p[vi[0]]+vi[2])*(p[vi[0]]+vi[2]+1))/2);
            for (vi[1]=0; vi[1]<p[vi[0]]; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = B[vi[8]+vi[2]*2] * E[vi[6]+vi[1]*2] / LR[0];
                for (vi[3]=1; vi[3]<2; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (B[vi[8]+vi[2]*2+vi[3]]*E[vi[6]+vi[1]*2+vi[3]]/LR[vi[3]]);
                }
            }
        }
        for (vi[2]=0; vi[2]<5; vi[2]++)
        {
            vi[4] = (vi[0]*28) + (((p[vi[0]]+vi[2])*(p[vi[0]]+vi[2]+1))/2) + p[vi[0]];
            for (vi[1]=0; vi[1]<vi[2]+1; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = B[vi[8]+vi[2]*2] * B[vi[8]+vi[1]*2] / LR[0];
                for (vi[3]=1; vi[3]<2; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (B[vi[8]+vi[2]*2+vi[3]]*B[vi[8]+vi[1]*2+vi[3]]/LR[vi[3]]);
                }
                if ((vi[2]>1) && (vi[2]==vi[1]))
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (1/LQ[vi[2]]);
                }
                vd[1] = (A[vi[9]+vi[2]*5]*vd[3]) + (A[vi[9]+vi[2]*5+1]*vd[5]);
                vd[2] = (A[vi[9]+vi[2]*5]*vd[5]) + (A[vi[9]+vi[2]*5+1]*vd[4]);
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (vd[1]*A[vi[9]+vi[1]*5  ]);
                GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (vd[2]*A[vi[9]+vi[1]*5+1]);
                for (vi[3]=2; vi[3]<5; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (A[vi[9]+vi[2]*5+vi[3]]*A[vi[9]+vi[1]*5+vi[3]]/LQ[vi[3]]);
                }
            }
        }
        if (vd[0] != Ref[2+(vi[0]*9)])
        {
            vd[0] = Ref[2+(vi[0]*9)];
            vd[1] = sin(vd[0]);
            vd[2] = cos(vd[0]);
            vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
            vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
            vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
        }
        vi[4] = (vi[0]*28) + ((p[vi[0]]*(p[vi[0]]+1))/2) + 2*p[vi[0]];
        GHG[vi[4]-p[vi[0]]] = GHG[vi[4]-p[vi[0]]] + vd[3];
        GHG[vi[4]+1] = GHG[vi[4]+1] + vd[5];
        GHG[vi[4]+2] = GHG[vi[4]+2] + vd[4];
    }
    for (vi[0]=1; vi[0]<40; vi[0]++)
    {
        if (vd[0] != Ref[-7+(vi[0]*9)])
        {
            vd[0] = Ref[-7+(vi[0]*9)];
            vd[1] = sin(vd[0]);
            vd[2] = cos(vd[0]);
            vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
            vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
            vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
        }
        vi[5] = (vi[0]-1) * 4;
        vi[6] = vi[0] * 4;
        vi[7] = (vi[0]-1) * 10;
        vi[8] = vi[0] * 25;
        vi[9] = 1120 + (vi[0]-1)*(7*7);
        for (vi[2]=0; vi[2]<p[vi[0]]; vi[2]++)
        {
            vi[4] = vi[9] + vi[2]*(5+p[vi[0]-1]);
            for (vi[1]=0; vi[1]<p[vi[0]-1]; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = F[vi[5]+vi[2]*2] * E[vi[5]+vi[1]*2] / LR[0];
                for (vi[3]=1; vi[3]<2; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (F[vi[5]+vi[2]*2+vi[3]]*E[vi[5]+vi[1]*2+vi[3]]/LR[vi[3]]);
                }
            }
        }
        for (vi[2]=0; vi[2]<p[vi[0]]; vi[2]++)
        {
            vi[4] = vi[9] + vi[2]*(5+p[vi[0]-1]) + p[vi[0]-1];
            for (vi[1]=0; vi[1]<5; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = F[vi[5]+vi[2]*2] * B[vi[7]+vi[1]*2] / LR[0];
                for (vi[3]=1; vi[3]<2; vi[3]++)
                {
                    GHG[vi[4]+vi[1]] = GHG[vi[4]+vi[1]] + (F[vi[5]+vi[2]*2+vi[3]]*B[vi[7]+vi[1]*2+vi[3]]/LR[vi[3]]);
                }
            }
        }
        for (vi[2]=0; vi[2]<5; vi[2]++)
        {
            vi[4] = vi[9] + (p[vi[0]]+vi[2])*(5+p[vi[0]-1]);
            for (vi[1]=0; vi[1]<p[vi[0]-1]; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = 0;
            }
            vi[4] = vi[9] + (p[vi[0]]+vi[2])*(5+p[vi[0]-1]) + p[vi[0]-1];
            GHG[vi[4]  ] = - (A[vi[8]+vi[2]*5]*vd[3]) - (A[vi[8]+vi[2]*5+1]*vd[5]);
            GHG[vi[4]+1] = - (A[vi[8]+vi[2]*5]*vd[5]) - (A[vi[8]+vi[2]*5+1]*vd[4]);
            for (vi[1]=2; vi[1]<5; vi[1]++)
            {
                GHG[vi[4]+vi[1]] = - A[vi[8]+vi[2]*5+vi[1]] / LQ[vi[1]];
            }
        }
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[9] = vi[0] * 28;
        vi[10] = 1120 + (vi[0]*(7*7));
        for (vi[2]=0; vi[2]<5+p[vi[0]]; vi[2]++)
        {
            vi[13] = vi[9] + ((vi[2]*(vi[2]+1))/2);
            vd[0] = 0;
            for (vi[4]=0; vi[4]<vi[2]; vi[4]++)
            {
                vd[0] = vd[0] - (L[vi[13]+vi[4]]*L[vi[13]+vi[4]]);
            }
            if (vi[0] > 0)
            {
                vi[12] = vi[10] - (7*7) + vi[2]*(5+p[vi[0]-1]);
                for (vi[4]=0; vi[4]<5+p[vi[0]-1]; vi[4]++)
                {
                    vd[0] = vd[0] - (L[vi[12]+vi[4]]*L[vi[12]+vi[4]]);
                }
            }
            vd[0] = vd[0] + GHG[vi[13]+vi[2]];
            if (vd[0] < 0)
            {
                L[vi[13]+vi[2]] = 0;
            }
            else
            {
                L[vi[13]+vi[2]] = sqrt(vd[0]);
            }
            for (vi[1]=vi[2]+1; vi[1]<5+p[vi[0]]; vi[1]++)
            {
                vi[3] = vi[9] + ((vi[1]*(vi[1]+1))/2);
                vd[0] = 0;
                for (vi[4]=0; vi[4]<vi[2]; vi[4]++)
                {
                    vd[0] = vd[0] - (L[vi[3]+vi[4]]*L[vi[13]+vi[4]]);
                }
                if (vi[0] > 0)
                {
                    vi[11] = vi[10] - (7*7) + vi[2]*(5+p[vi[0]-1]);
                    vi[12] = vi[10] - (7*7) + vi[1]*(5+p[vi[0]-1]);
                    for (vi[4]=0; vi[4]<5+p[vi[0]-1]; vi[4]++)
                    {
                        vd[0] = vd[0] - (L[vi[11]+vi[4]]*L[vi[12]+vi[4]]);
                    }
                }
                vd[0] = vd[0] + GHG[vi[3]+vi[2]];
                if (L[vi[13]+vi[2]] == 0)
                {
                    L[vi[3]+vi[2]] = 0.0;
                }
                else
                {
                    L[vi[3]+vi[2]] = vd[0] / L[vi[13]+vi[2]];
                }
            }
            if (vi[0] < 39)
            {
                for (vi[1]=0; vi[1]<5+p[vi[0]+1]; vi[1]++)
                {
                    vi[3] = vi[10] + (vi[1]*(5+p[vi[0]]));
                    vd[0] = 0;
                    for (vi[4]=0; vi[4]<vi[2]; vi[4]++)
                    {
                        vd[0] = vd[0] - (L[vi[3]+vi[4]]*L[vi[13]+vi[4]]);
                    }
                    vd[0] = vd[0] + GHG[vi[3]+vi[2]];
                    if (L[vi[13]+vi[2]] == 0)
                    {
                        L[vi[3]+vi[2]] = 0;
                    }
                    else
                    {
                        L[vi[3]+vi[2]] = vd[0] / L[vi[13]+vi[2]];
                    }
                }
            }
        }
    }
    for (vi[2]=0; vi[2]<5+p[0]; vi[2]++)
    {
        vi[13] = (vi[2]*(vi[2]+1)) / 2;
        vd[0] = ghf[vi[2]];
        for (vi[1]=0; vi[1]<vi[2]; vi[1]++)
        {
            vd[0] = vd[0] - (L[vi[13]+vi[1]]*lambda[vi[1]]);
        }
        if (L[vi[13]+vi[2]] == 0)
        {
            lambda[vi[2]] = 0;
        }
        else
        {
            lambda[vi[2]] = vd[0] / L[vi[13]+vi[2]];
        }
    }
    for (vi[0]=1; vi[0]<40; vi[0]++)
    {
        vi[7] = (vi[0]-1) * 7;
        vi[8] = vi[0] * 7;
        vi[9] = vi[0] * 28;
        vi[10] = 1120 + ((vi[0]-1)*(7*7));
        for (vi[2]=0; vi[2]<5+p[vi[0]]; vi[2]++)
        {
            vi[4] = vi[10] + (vi[2]*(5+p[vi[0]-1]));
            vd[0] = ghf[vi[8]+vi[2]];
            for (vi[1]=0; vi[1]<5+p[vi[0]-1]; vi[1]++)
            {
                vd[0] = vd[0] - (L[vi[4]+vi[1]]*lambda[vi[7]+vi[1]]);
            }
            vi[13] = vi[9] + ((vi[2]*(vi[2]+1))/2);
            for (vi[1]=0; vi[1]<vi[2]; vi[1]++)
            {
                vd[0] = vd[0] - (L[vi[13]+vi[1]]*lambda[vi[8]+vi[1]]);
            }
            if (L[vi[13]+vi[2]] == 0)
            {
                lambda[vi[8]+vi[2]] = 0;
            }
            else
            {
                lambda[vi[8]+vi[2]] = vd[0] / L[vi[13]+vi[2]];
            }
        }
    }
    for (vi[2]=5+p[39]-1; vi[2]>=0; vi[2]--)
    {
        vi[13] = vi[9] + vi[2];
        vd[0] = 0;
        for (vi[1]=vi[2]+1; vi[1]<5+p[39]; vi[1]++)
        {
            vd[0] = vd[0] - (L[vi[13]+((vi[1]*(vi[1]+1))/2)]*lambda[vi[8]+vi[1]]);
        }
        vd[0] = vd[0] + lambda[vi[8]+vi[2]];
        if (L[vi[13]+((vi[2]*(vi[2]+1))/2)] == 0)
        {
            lambda[vi[8]+vi[2]] = 0;
        }
        else
        {
            lambda[vi[8]+vi[2]] = vd[0] / L[vi[13]+((vi[2]*(vi[2]+1))/2)];
        }
    }
    for (vi[0]=38; vi[0]>=0; vi[0]--)
    {
        vi[9] = vi[0] * 28;
        vi[10] = 1120 + (vi[0]*(7*7));
        vi[7] = vi[0] * 7;
        vi[8] = (vi[0]+1) * 7;
        for (vi[2]=5+p[vi[0]]-1; vi[2]>=0; vi[2]--)
        {
            vi[4] = vi[10] + vi[2];
            vd[0] = 0;
            for (vi[1]=0; vi[1]<5+p[vi[0]+1]; vi[1]++)
            {
                vd[0] = vd[0] - (L[vi[4]+vi[1]*(5+p[vi[0]])]*lambda[vi[8]+vi[1]]);
            }
            vi[13] = vi[9] + vi[2];
            for (vi[1]=vi[2]+1; vi[1]<5+p[vi[0]]; vi[1]++)
            {
                vd[0] = vd[0] - (L[vi[13]+((vi[1]*(vi[1]+1))/2)]*lambda[vi[7]+vi[1]]);
            }
            vd[0] = vd[0] + lambda[vi[7]+vi[2]];
            if (L[vi[13]+((vi[2]*(vi[2]+1))/2)] == 0)
            {
                lambda[vi[7]+vi[2]] = 0;
            }
            else
            {
                lambda[vi[7]+vi[2]] = vd[0] / L[vi[13]+((vi[2]*(vi[2]+1))/2)];
            }
        }
    }
    vi[3] = 0;
    while (vi[3]<1)
    {
        vi[3]++;
        for (vi[0]=0; vi[0]<40; vi[0]++)
        {
            vi[7] = (vi[0]-1) * 7;
            vi[12] = vi[0] * 7;
            vi[8] = (vi[0]+1) * 7;
            for (vi[2]=0; vi[2]<5+p[vi[0]]; vi[2]++)
            {
                vi[4] = vi[12] + vi[2];
                x[vi[4]] = - ghf[vi[4]];
                vd[4] = 0;
                vd[5] = 0;
                vd[6] = 0;
                if (vi[0] > 0)
                {
                    vi[11] = 1120 + (vi[0]-1)*(7*7) + vi[2]*(5+p[vi[0]-1]);
                    for (vi[1]=0; vi[1]<5+p[vi[0]-1]; vi[1]++)
                    {
                        vd[0] = GHG[vi[11]+vi[1]] * lambda[vi[7]+vi[1]];
                        vd[1] = vd[0] + x[vi[4]];
                        if (fabs(vd[0]) > fabs(x[vi[4]]))
                        {
                            vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                        }
                        else
                        {
                            vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                        }
                        x[vi[4]] = vd[1];
                        vd[1] = vd[4] + vd[2];
                        if (fabs(vd[2]) > fabs(vd[4]))
                        {
                            vd[2] = (vd[2] - vd[1]) + vd[4];
                        }
                        else
                        {
                            vd[2] = (vd[4] - vd[1]) + vd[2];
                        }
                        vd[4] = vd[1];
                        vd[1] = vd[5] + vd[2];
                        if (fabs(vd[2]) > fabs(vd[5]))
                        {
                            vd[2] = (vd[2] - vd[1]) + vd[5];
                        }
                        else
                        {
                            vd[2] = (vd[5] - vd[1]) + vd[2];
                        }
                        vd[5] = vd[1];
                        vd[6] = vd[6] + vd[2];
                    }
                }
                if (vi[0] < 39)
                {
                    vi[11] = 1120 + (vi[0]*(7*7)) + vi[2];
                    for (vi[1]=0; vi[1]<5+p[vi[0]+1]; vi[1]++)
                    {
                        vd[0] = GHG[vi[11]+vi[1]*(5+p[vi[0]])] * lambda[vi[8]+vi[1]];
                        vd[1] = vd[0] + x[vi[4]];
                        if (fabs(vd[0]) > fabs(x[vi[4]]))
                        {
                            vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                        }
                        else
                        {
                            vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                        }
                        x[vi[4]] = vd[1];
                        vd[1] = vd[4] + vd[2];
                        if (fabs(vd[2]) > fabs(vd[4]))
                        {
                            vd[2] = (vd[2] - vd[1]) + vd[4];
                        }
                        else
                        {
                            vd[2] = (vd[4] - vd[1]) + vd[2];
                        }
                        vd[4] = vd[1];
                        vd[1] = vd[5] + vd[2];
                        if (fabs(vd[2]) > fabs(vd[5]))
                        {
                            vd[2] = (vd[2] - vd[1]) + vd[5];
                        }
                        else
                        {
                            vd[2] = (vd[5] - vd[1]) + vd[2];
                        }
                        vd[5] = vd[1];
                        vd[6] = vd[6] + vd[2];
                    }
                }
                vi[13] = (vi[0]*28) + ((vi[2]*(vi[2]+1))/2);
                for (vi[1]=0; vi[1]<vi[2]; vi[1]++)
                {
                    vd[0] = GHG[vi[13]+vi[1]] * lambda[vi[12]+vi[1]];
                    vd[1] = vd[0] + x[vi[4]];
                    if (fabs(vd[0]) > fabs(x[vi[4]]))
                    {
                        vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                    }
                    else
                    {
                        vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                    }
                    x[vi[4]] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
                vi[13] = (vi[0]*28) + vi[2];
                for (vi[1]=vi[2]; vi[1]<5+p[vi[0]]; vi[1]++)
                {
                    vd[0] = GHG[vi[13]+((vi[1]*(vi[1]+1))/2)] * lambda[vi[12]+vi[1]];
                    vd[1] = vd[0] + x[vi[4]];
                    if (fabs(vd[0]) > fabs(x[vi[4]]))
                    {
                        vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                    }
                    else
                    {
                        vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                    }
                    x[vi[4]] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
            }
        }
        for (vi[2]=0; vi[2]<5+p[0]; vi[2]++)
        {
            vi[13] = (vi[2]*(vi[2]+1)) / 2;
            vd[3] = x[vi[2]];
            vd[4] = 0;
            vd[5] = 0;
            vd[6] = 0;
            for (vi[1]=0; vi[1]<vi[2]; vi[1]++)
            {
                vd[0] = - L[vi[13]+vi[1]] * x[vi[1]];
                vd[1] = vd[0] + vd[3];
                if (fabs(vd[0]) > fabs(vd[3]))
                {
                    vd[2] = (vd[0] - vd[1]) + vd[3];
                }
                else
                {
                    vd[2] = (vd[3] - vd[1]) + vd[0];
                }
                vd[3] = vd[1];
                vd[1] = vd[4] + vd[2];
                if (fabs(vd[2]) > fabs(vd[4]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[4];
                }
                else
                {
                    vd[2] = (vd[4] - vd[1]) + vd[2];
                }
                vd[4] = vd[1];
                vd[1] = vd[5] + vd[2];
                if (fabs(vd[2]) > fabs(vd[5]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[5];
                }
                else
                {
                    vd[2] = (vd[5] - vd[1]) + vd[2];
                }
                vd[5] = vd[1];
                vd[6] = vd[6] + vd[2];
            }
            if (L[vi[13]+vi[2]] == 0)
            {
                x[vi[2]] = 0;
            }
            else
            {
                x[vi[2]] = vd[3] / L[vi[13]+vi[2]];
            }
        }
        for (vi[0]=1; vi[0]<40; vi[0]++)
        {
            vi[7] = (vi[0]-1) * 7;
            vi[8] = vi[0] * 7;
            vi[9] = vi[0] * 28;
            vi[10] = 1120 + ((vi[0]-1)*(7*7));
            for (vi[2]=0; vi[2]<5+p[vi[0]]; vi[2]++)
            {
                vi[4] = vi[10] + (vi[2]*(5+p[vi[0]-1]));
                vd[3] = x[vi[8]+vi[2]];
                vd[4] = 0;
                vd[5] = 0;
                vd[6] = 0;
                for (vi[1]=0; vi[1]<5+p[vi[0]-1]; vi[1]++)
                {
                    vd[0] = - L[vi[4]+vi[1]] * x[vi[7]+vi[1]];
                    vd[1] = vd[0] + vd[3];
                    if (fabs(vd[0]) > fabs(vd[3]))
                    {
                        vd[2] = (vd[0] - vd[1]) + vd[3];
                    }
                    else
                    {
                        vd[2] = (vd[3] - vd[1]) + vd[0];
                    }
                    vd[3] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
                vi[13] = vi[9] + ((vi[2]*(vi[2]+1))/2);
                for (vi[1]=0; vi[1]<vi[2]; vi[1]++)
                {
                    vd[0] = - L[vi[13]+vi[1]] * x[vi[8]+vi[1]];
                    vd[1] = vd[0] + vd[3];
                    if (fabs(vd[0]) > fabs(vd[3]))
                    {
                        vd[2] = (vd[0] - vd[1]) + vd[3];
                    }
                    else
                    {
                        vd[2] = (vd[3] - vd[1]) + vd[0];
                    }
                    vd[3] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
                if (L[vi[13]+vi[2]] == 0)
                {
                    x[vi[8]+vi[2]] = 0;
                }
                else
                {
                    x[vi[8]+vi[2]] = vd[3] / L[vi[13]+vi[2]];
                }
            }
        }
        for (vi[2]=5+p[39]-1; vi[2]>=0; vi[2]--)
        {
            vi[13] = vi[9] + vi[2];
            vd[3] = x[vi[8]+vi[2]];
            vd[4] = 0;
            vd[5] = 0;
            vd[6] = 0;
            for (vi[1]=vi[2]+1; vi[1]<5+p[39]; vi[1]++)
            {
                vd[0] = - L[vi[13]+((vi[1]*(vi[1]+1))/2)] * x[vi[8]+vi[1]];
                vd[1] = vd[0] + vd[3];
                if (fabs(vd[0]) > fabs(vd[3]))
                {
                    vd[2] = (vd[0] - vd[1]) + vd[3];
                }
                else
                {
                    vd[2] = (vd[3] - vd[1]) + vd[0];
                }
                vd[3] = vd[1];
                vd[1] = vd[4] + vd[2];
                if (fabs(vd[2]) > fabs(vd[4]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[4];
                }
                else
                {
                    vd[2] = (vd[4] - vd[1]) + vd[2];
                }
                vd[4] = vd[1];
                vd[1] = vd[5] + vd[2];
                if (fabs(vd[2]) > fabs(vd[5]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[5];
                }
                else
                {
                    vd[2] = (vd[5] - vd[1]) + vd[2];
                }
                vd[5] = vd[1];
                vd[6] = vd[6] + vd[2];
            }
            if (L[vi[13]+((vi[2]*(vi[2]+1))/2)] == 0)
            {
                x[vi[8]+vi[2]] = 0;
            }
            else
            {
                x[vi[8]+vi[2]] = vd[3] / L[vi[13]+((vi[2]*(vi[2]+1))/2)];
            }
            lambda[vi[8]+vi[2]] = lambda[vi[8]+vi[2]] - x[vi[8]+vi[2]];
        }
        for (vi[0]=38; vi[0]>=0; vi[0]--)
        {
            vi[9] = vi[0] * 28;
            vi[10] = 1120 + (vi[0]*(7*7));
            vi[7] = vi[0]*7;
            vi[8] = (vi[0]+1)*7;
            for (vi[2]=5+p[vi[0]]-1; vi[2]>=0; vi[2]--)
            {
                vi[4] = vi[10] + vi[2];
                vd[3] = x[vi[7]+vi[2]];
                vd[4] = 0;
                vd[5] = 0;
                vd[6] = 0;
                for (vi[1]=0; vi[1]<5+p[vi[0]+1]; vi[1]++)
                {
                    vd[0] = - L[vi[4]+vi[1]*(5+p[vi[0]])] * x[vi[8]+vi[1]];
                    vd[1] = vd[0] + vd[3];
                    if (fabs(vd[0]) > fabs(vd[3]))
                    {
                        vd[2] = (vd[0] - vd[1]) + vd[3];
                    }
                    else
                    {
                        vd[2] = (vd[3] - vd[1]) + vd[0];
                    }
                    vd[3] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
                vi[13] = vi[9] + vi[2];
                for (vi[1]=vi[2]+1; vi[1]<5+p[vi[0]]; vi[1]++)
                {
                    vd[0] = - L[vi[13]+((vi[1]*(vi[1]+1))/2)] * x[vi[7]+vi[1]];
                    vd[1] = vd[0] + vd[3];
                    if (fabs(vd[0]) > fabs(vd[3]))
                    {
                        vd[2] = (vd[0] - vd[1]) + vd[3];
                    }
                    else
                    {
                        vd[2] = (vd[3] - vd[1]) + vd[0];
                    }
                    vd[3] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
                if (L[vi[13]+((vi[2]*(vi[2]+1))/2)] == 0)
                {
                    x[vi[7]+vi[2]] = 0;
                }
                else
                {
                    x[vi[7]+vi[2]] = vd[3] / L[vi[13]+((vi[2]*(vi[2]+1))/2)];
                }
                lambda[vi[7]+vi[2]] = lambda[vi[7]+vi[2]] - x[vi[7]+vi[2]];
            }
        }
    }
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[7] = vi[0] * 7;
        vi[8] = (vi[0]+1) * 7;
        for (vi[2]=0; vi[2]<2; vi[2]++)
        {
            vi[4] = (vi[0]*7) + vi[2];
            x[vi[4]] = fx[vi[4]];
            vd[4] = 0;
            vd[5] = 0;
            vd[6] = 0;
            vi[11] = vi[0]*4 + vi[2];
            for (vi[1]=0; vi[1]<p[vi[0]]; vi[1]++)
            {
                vd[0] = E[vi[11]+vi[1]*2] * lambda[vi[7]+vi[1]];
                vd[1] = vd[0] + x[vi[4]];
                if (fabs(vd[0]) > fabs(x[vi[4]]))
                {
                    vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                }
                else
                {
                    vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                }
                x[vi[4]] = vd[1];
                vd[1] = vd[4] + vd[2];
                if (fabs(vd[2]) > fabs(vd[4]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[4];
                }
                else
                {
                    vd[2] = (vd[4] - vd[1]) + vd[2];
                }
                vd[4] = vd[1];
                vd[1] = vd[5] + vd[2];
                if (fabs(vd[2]) > fabs(vd[5]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[5];
                }
                else
                {
                    vd[2] = (vd[5] - vd[1]) + vd[2];
                }
                vd[5] = vd[1];
                vd[6] = vd[6] + vd[2];
            }
            if (vi[0] < 39)
            {
                vi[11] = (vi[0]*4) + vi[2];
                for (vi[1]=0; vi[1]<p[vi[0]+1]; vi[1]++)
                {
                    vd[0] = F[vi[11]+vi[1]*2] * lambda[vi[8]+vi[1]];
                    vd[1] = vd[0] + x[vi[4]];
                    if (fabs(vd[0]) > fabs(x[vi[4]]))
                    {
                        vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                    }
                    else
                    {
                        vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                    }
                    x[vi[4]] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
            }
            vi[11] = (vi[0]*10) + vi[2];
            for (vi[1]=0; vi[1]<5; vi[1]++)
            {
                vd[0] = B[vi[11]+vi[1]*2] * lambda[vi[7]+p[vi[0]]+vi[1]];
                vd[1] = vd[0] + x[vi[4]];
                if (fabs(vd[0]) > fabs(x[vi[4]]))
                {
                    vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                }
                else
                {
                    vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                }
                x[vi[4]] = vd[1];
                vd[1] = vd[4] + vd[2];
                if (fabs(vd[2]) > fabs(vd[4]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[4];
                }
                else
                {
                    vd[2] = (vd[4] - vd[1]) + vd[2];
                }
                vd[4] = vd[1];
                vd[1] = vd[5] + vd[2];
                if (fabs(vd[2]) > fabs(vd[5]))
                {
                    vd[2] = (vd[2] - vd[1]) + vd[5];
                }
                else
                {
                    vd[2] = (vd[5] - vd[1]) + vd[2];
                }
                vd[5] = vd[1];
                vd[6] = vd[6] + vd[2];
            }
        }
        for (vi[2]=0; vi[2]<5; vi[2]++)
        {
            vi[4] = vi[0]*7 + 2 + vi[2];
            x[vi[4]] = fx[vi[4]] - lambda[vi[7]+p[vi[0]]+vi[2]];
            if (vi[0] < 39)
            {
                vi[11] = ((vi[0]+1)*25) + vi[2];
                for (vi[1]=0; vi[1]<5; vi[1]++)
                {
                    vd[0] = A[vi[11]+vi[1]*5] * lambda[vi[8]+p[vi[0]+1]+vi[1]];
                    vd[1] = vd[0] + x[vi[4]];
                    if (fabs(vd[0]) > fabs(x[vi[4]]))
                    {
                        vd[2] = (vd[0] - vd[1]) + x[vi[4]];
                    }
                    else
                    {
                        vd[2] = (x[vi[4]] - vd[1]) + vd[0];
                    }
                    x[vi[4]] = vd[1];
                    vd[1] = vd[4] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[4]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[4];
                    }
                    else
                    {
                        vd[2] = (vd[4] - vd[1]) + vd[2];
                    }
                    vd[4] = vd[1];
                    vd[1] = vd[5] + vd[2];
                    if (fabs(vd[2]) > fabs(vd[5]))
                    {
                        vd[2] = (vd[2] - vd[1]) + vd[5];
                    }
                    else
                    {
                        vd[2] = (vd[5] - vd[1]) + vd[2];
                    }
                    vd[5] = vd[1];
                    vd[6] = vd[6] + vd[2];
                }
            }
        }
    }
    vd[0] = Ref[2];
    vd[1] = sin(vd[0]);
    vd[2] = cos(vd[0]);
    vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
    vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
    vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        if (vd[0] != Ref[2+(vi[0]*9)])
        {
            vd[0] = Ref[2+(vi[0]*9)];
            vd[1] = sin(vd[0]);
            vd[2] = cos(vd[0]);
            vd[3] = (vd[1]*vd[1]/LQ[0]) + (vd[2]*vd[2]/LQ[1]);
            vd[4] = (vd[2]*vd[2]/LQ[0]) + (vd[1]*vd[1]/LQ[1]);
            vd[5] = ((1/LQ[1])-(1/LQ[0])) * vd[1] * vd[2];
        }
        vi[4] = vi[0] * 7;
        for (vi[2]=0; vi[2]<2; vi[2]++)
        {
            x[vi[4]+vi[2]] = - x[vi[4]+vi[2]] / LR[vi[2]];
        }
        vi[4] = (vi[0]*7) + 2;
        vd[1]      = - (x[vi[4]]*vd[3]) - (x[vi[4]+1]*vd[5]);
        x[vi[4]+1] = - (x[vi[4]]*vd[5]) - (x[vi[4]+1]*vd[4]);
        x[vi[4]  ] = vd[1];
        for (vi[2]=2; vi[2]<5; vi[2]++)
        {
            x[vi[4]+vi[2]] = - x[vi[4]+vi[2]] / LQ[vi[2]];
        }
    }
    x[280] = 1;
    for (vi[0]=0; vi[0]<280; vi[0]++)
    {
        if      (x[vi[0]] >   x[280])
        {
            x[280] =   x[vi[0]];
        }
        else if (x[vi[0]] < - x[280])
        {
            x[280] = - x[vi[0]];
        }
    }
}
 
static void gsmodx(const int8_t *Uas, const int8_t *dUas, double *x, int64_t *vi)
{
    for (vi[0]=0; vi[0]<2; vi[0]++)
    {
        vi[1] = vi[0];
        vi[2] = vi[0];
        while (vi[1] < 80)
        {
            if ((Uas[vi[1]] < -3) || (Uas[vi[1]] > +3))
            {
                x[vi[2]] = 0;
                if (vi[1] < 78)
                {
                    if ((dUas[vi[1]]<-1) || (dUas[vi[1]]>+1))
                    {
                        vi[3] = 2;
                    }
                    else
                    {
                        vi[3] = 0;
                    }
                }
                else
                {
                    vi[3] = 0;
                }
                while (vi[3] > 1)
                {
                    vi[1] = vi[1] + 2;
                    vi[2] = vi[2] + 7;
                    x[vi[2]] = 0;
                    if (vi[1] < 78)
                    {
                        if ((dUas[vi[1]]<-1) || (dUas[vi[1]]>+1))
                        {
                            vi[3] = 2;
                        }
                        else
                        {
                            vi[3] = 0;
                        }
                    }
                    else
                    {
                        vi[3] = 0;
                    }
                }
            }
            else if (vi[1] < 78)
            {
                vi[3] = 0;
                vi[4] = 4;
                if ((dUas[vi[1]]<-1) || (dUas[vi[1]]>+1))
                {
                    vi[3] = 2;
                    vi[4] = 1;
                    vi[5] = vi[2];
                    vi[6] = 0;
                    if ((Uas[vi[1]]<-1) || (Uas[vi[1]]>+1))
                    {
                        vi[6] = 2;
                    }
                    while (vi[3] > 1)
                    {
                        vi[1] = vi[1] + 2;
                        vi[2] = vi[2] + 7;
                        vi[3] = 0;
                        vi[4]++;
                        x[vi[5]] = x[vi[5]] + x[vi[2]];
                        if (vi[1] < 78)
                        {
                            if ((dUas[vi[1]]<-1) || (dUas[vi[1]]>+1))
                            {
                                vi[3] = 2;
                            }
                            if ((Uas[vi[1]]<-1) || (Uas[vi[1]]>+1))
                            {
                                vi[6] = 2;
                            }
                        }
                    }
                    if (vi[4] > 0)
                    {
                        if (vi[6] > 1)
                        {
                            x[vi[5]] = 0;
                        }
                        else
                        {
                            x[vi[5]] = x[vi[5]] / vi[4];
                        }
                        while (vi[4] > 1)
                        {
                            vi[4]--;
                            x[vi[5]+vi[4]*7] = x[vi[5]];
                        }
                    }
                }
            }
            vi[1] = vi[1] + 2;
            vi[2] = vi[2] + 7;
        }
    }
    x[280] = 1;
    for (vi[0]=0; vi[0]<280; vi[0]++)
    {
        if      (x[vi[0]] >   x[280])
        {
            x[280] =   x[vi[0]];
        }
        else if (x[vi[0]] < - x[280])
        {
            x[280] = - x[vi[0]];
        }
    }
}
 
static void gsline(double *U, double *X, int8_t *Uas, int8_t *dUas, const double *UconMod, const double *Ref, const uint8_t *drivmode, const double *R, const double *Q, const double *contolerance, const double *conpenalty, double *x, double *DJ, int16_t *newcon, double *alphasum, int64_t *vi, double *vd)
{
    vd[5] = 1 - (*alphasum);
    *newcon = 0;
    if (x[280] == 0)
    {
        DJ[0] = 0;
        DJ[1] = 0;
        DJ[2] = 0;
        DJ[3] = 0;
        return;
    }
    else
    {
        vd[7] = 1.00000000000000005e-04 / x[280];
    }
    for (vi[1]=0; vi[1]<2; vi[1]++)
    {
        if ((Uas[vi[1]]>-1) && (Uas[vi[1]]<+1))
        {
            if (x[vi[1]] < 0)
            {
                if (UconMod[vi[1]] > (U[vi[1]] + UconMod[4+vi[1]]))
                {
                    vd[0] = (UconMod[vi[1]] - U[2+vi[1]]) / x[vi[1]];
                }
                else
                {
                    vd[0] = (U[vi[1]] + UconMod[4+vi[1]] - U[2+vi[1]]) / x[vi[1]];
                }
                if (vd[0] < vd[5])
                {
                    vd[5] = vd[0];
                    *newcon = -(vi[1]+1);
                }
            }
            if (x[vi[1]] > 0)
            {
                if (UconMod[2+vi[1]] < (U[vi[1]] + UconMod[6+vi[1]]))
                {
                    vd[0] = (UconMod[2+vi[1]] - U[2+vi[1]]) / x[vi[1]];
                }
                else
                {
                    vd[0] = (U[vi[1]] + UconMod[6+vi[1]] - U[2+vi[1]]) / x[vi[1]];
                }
                if (vd[0] < vd[5])
                {
                    vd[5] = vd[0];
                    *newcon =  (vi[1]+1);
                }
            }
        }
    }
    for (vi[0]=0; vi[0]<39; vi[0]++)
    {
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            vi[2] = ((vi[0]+1)*2)      + vi[1];
            vi[3] = ((vi[0]+2)*2)      + vi[1];
            vi[4] =     (vi[0]*7) + vi[1];
            vi[5] = ((vi[0]+1)*7) + vi[1];
            if ((Uas[vi[2]]>-1) && (Uas[vi[2]]<+1))
            {
                if      (x[vi[5]] < 0)
                {
                    vd[0] = (UconMod[       vi[1]] - U[vi[3]]) / x[vi[5]];
                    if (vd[0] < vd[5])
                    {
                        vd[5] = vd[0];
                        *newcon = -(vi[2]+1);
                    }
                }
                else if (x[vi[5]] > 0)
                {
                    vd[0] = (UconMod[2+vi[1]] - U[vi[3]]) / x[vi[5]];
                    if (vd[0] < vd[5])
                    {
                        vd[5] = vd[0];
                        *newcon =  (vi[2]+1);
                    }
                }
            }
            if ((dUas[(vi[0]*2)+vi[1]]>-1) && (dUas[(vi[0]*2)+vi[1]]<+1))
            {
                if      ((x[vi[5]] - x[vi[4]]) < 0)
                {
                    vd[0] = (UconMod[4+vi[1]] - U[vi[3]]+U[vi[2]]) / (x[vi[5]] - x[vi[4]]);
                    if (vd[0] < vd[5])
                    {
                        vd[5] = vd[0];
                        *newcon = -80 - ((vi[0]*2)+vi[1]+1);
                    }
                }
                else if ((x[vi[5]] - x[vi[4]]) > 0)
                {
                    vd[0] = (UconMod[6+vi[1]] - U[vi[3]]+U[vi[2]]) / (x[vi[5]] - x[vi[4]]);
                    if (vd[0] < vd[5])
                    {
                        vd[5] = vd[0];
                        *newcon =  80 + ((vi[0]*2)+vi[1]+1);
                    }
                }
            }
        }
    }
    if (vd[5] <= vd[7])
    {
        DJ[0] = 0;
        DJ[1] = 0;
        DJ[2] = 0;
        DJ[3] = 0;
        return;
    }
    gseval(U,X,x,&vd[7],Ref,drivmode,R,Q,contolerance,conpenalty,&vd[1],&vi[7],&vd[8]);
    if (((vd[1]>0) || ((vd[1]==0)&&(vd[2]>0))) || ((((vd[1]==0)&&(vd[2]==0))&&(vd[3]>0)) || (((vd[1]==0)&&(vd[2]==0))&&((vd[3]==0)&&(vd[4]>0)))))
    {
        DJ[0] = 0;
        DJ[1] = 0;
        DJ[2] = 0;
        DJ[3] = 0;
        *newcon = 0;
        *alphasum = *alphasum + 5;
        return;
    }
    else
    {
        vd[0] = vd[7];
        DJ[0] = vd[1];
        DJ[1] = vd[2];
        DJ[2] = vd[3];
        DJ[3] = vd[4];
        vd[6] = vd[1] / vd[7];
    }
    gseval(U,X,x,&vd[5],Ref,drivmode,R,Q,contolerance,conpenalty,&vd[1],&vi[7],&vd[8]);
    if (((vd[1]<DJ[0]) || ((vd[1]==DJ[0])&&(vd[2]<DJ[1]))) || ((((vd[1]==DJ[0])&&(vd[2]==DJ[1]))&&(vd[3]<DJ[2])) || (((vd[1]==DJ[0])&&(vd[2]==DJ[1]))&&((vd[3]==DJ[2])&&(vd[4]<DJ[3])))))
    {
        vd[0] = vd[5];
        DJ[0] = vd[1];
        DJ[1] = vd[2];
        DJ[2] = vd[3];
        DJ[3] = vd[4];
    }
    while (vd[1] >= vd[6]*vd[5]*5.99999999999999978e-01)
    {
        vd[5] = 5.00000000000000000e-01 * vd[5];
        gseval(U,X,x,&vd[5],Ref,drivmode,R,Q,contolerance,conpenalty,&vd[1],&vi[7],&vd[8]);
        if (((vd[1]<DJ[0]) || ((vd[1]==DJ[0])&&(vd[2]<DJ[1]))) || ((((vd[1]==DJ[0])&&(vd[2]==DJ[1]))&&(vd[3]<DJ[2])) || (((vd[1]==DJ[0])&&(vd[2]==DJ[1]))&&((vd[3]==DJ[2])&&(vd[4]<DJ[3])))))
        {
            vd[0] = vd[5];
            DJ[0] = vd[1];
            DJ[1] = vd[2];
            DJ[2] = vd[3];
            DJ[3] = vd[4];
            *newcon = 0;
        }
        if (vd[5] < vd[7])
        {
            break;
        }
    }
    *alphasum = *alphasum + vd[0];
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        for (vi[1]=0; vi[1]<2; vi[1]++)
        {
            vi[3] = ((vi[0]+1)*2) + vi[1];
            vi[6] = (vi[0]*7) + vi[1];
            U[vi[3]] = U[vi[3]] + (vd[0]*x[vi[6]]);
        }
    }
}
 
static void gszeroxfw(const int8_t *Uas, const int8_t *dUas, const int64_t *ind, double *x, int64_t *vi)
{
    vi[0] = *ind;
    vi[1] = (int64_t)(vi[0]/2);
    vi[1] = vi[1]*5 + vi[0];
    if      ((vi[1]>=0) && (vi[1]<280))
    {
        x[vi[1]] = 0;
    }
    while (vi[0] < 78)
    {
        if ((dUas[vi[0]]<-1) || (dUas[vi[0]]>+1))
        {
            vi[0] = vi[0] + 2;
            vi[1] = vi[1] + 7;
            if ((vi[1]>=0) && (vi[1]<280))
            {
                x[vi[1]] = 0;
            }
        }
        else
        {
            break;
        }
    }
}
 
static void gsavgxfw(const int8_t *Uas, const int8_t *dUas, const int64_t *ind, double *x, int64_t *vi, double *vd)
{
    vi[0] = *ind;
    vi[1] = (int64_t)(vi[0]/2);
    vi[1] = vi[1]*5 + vi[0];
    vi[2] = 0;
    vd[0] = 0;
    if      ((vi[1]>=0) && (vi[1]<280))
    {
        vd[0] = vd[0] + x[vi[1]];
        vi[2]++;
    }
    while (vi[0] < 78)
    {
        if ((dUas[vi[0]]<-1) || (dUas[vi[0]]>+1))
        {
            vi[0] = vi[0] + 2;
            vi[1] = vi[1] + 7;
            if ((vi[1]>=0) && (vi[1]<280))
            {
                vd[0] = vd[0] + x[vi[1]];
                vi[2]++;
            }
        }
        else
        {
            break;
        }
    }
    if (vi[2] > 0)
    {
        vd[0] = vd[0] / vi[2];
    }
    else
    {
        vd[0] = 0;
    }
    vi[0] = *ind;
    vi[1] = (int64_t)(vi[0]/2);
    vi[1] = vi[1]*5 + vi[0];
    if      ((vi[1]>=0) && (vi[1]<280))
    {
        x[vi[1]] = vd[0];
    }
    while (vi[0] < 78)
    {
        if ((dUas[vi[0]]<-1) || (dUas[vi[0]]>+1))
        {
            vi[0] = vi[0] + 2;
            vi[1] = vi[1] + 7;
            if ((vi[1]>=0) && (vi[1]<280))
            {
                x[vi[1]] = vd[0];
            }
        }
        else
        {
            break;
        }
    }
}
 
static void gsactadd(int8_t *Uas, int8_t *dUas, double *x, int16_t *newcon, int64_t *vi, double *vd)
{
    if      ((*newcon>-81) && (*newcon<0))
    {
        vi[0] = -(*newcon) - 1;
        if (vi[0] < 2)
        {
            Uas[vi[0]] = -6;
            gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
        }
        else
        {
            if ((dUas[vi[0]-2]>-1) && (dUas[vi[0]-2]<+1))
            {
                Uas[vi[0]] = -6;
                gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
            }
            else
            {
                Uas[vi[0]] = -2;
                while (vi[0] >= 2)
                {
                    vi[0] = vi[0] - 2;
                    if ((dUas[vi[0]]>-1) && (dUas[vi[0]]<+1))
                    {
                        vi[1] = vi[0] + 2;
                        Uas[vi[1]] = -4;
                        gszeroxfw(&Uas[0],&dUas[0],&vi[1],&x[0],&vi[2]);
                        break;
                    }
                }
                if (vi[0] < 2)
                {
                    if ((dUas[vi[0]]<-1) || (dUas[vi[0]]>+1))
                    {
                        Uas[vi[0]] = -4;
                        gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
                    }
                }
            }
        }
    }
    else if ((*newcon< 81) && (*newcon>0))
    {
        vi[0] =  (*newcon) - 1;
        if (vi[0] < 2)
        {
            Uas[vi[0]] = +6;
            gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
        }
        else
        {
            if ((dUas[vi[0]-2]>-1) && (dUas[vi[0]-2]<+1))
            {
                Uas[vi[0]] = +6;
                gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
            }
            else
            {
                Uas[vi[0]] = +2;
                while (vi[0] >= 2)
                {
                    vi[0] = vi[0] - 2;
                    if ((dUas[vi[0]]>-1) && (dUas[vi[0]]<+1))
                    {
                        vi[1] = vi[0] + 2;
                        Uas[vi[1]] = +4;
                        gszeroxfw(&Uas[0],&dUas[0],&vi[1],&x[0],&vi[2]);
                        break;
                    }
                }
                if (vi[0] < 2)
                {
                    if ((dUas[vi[0]]<-1) || (dUas[vi[0]]>+1))
                    {
                        Uas[vi[0]] = +4;
                        gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
                    }
                }
            }
        }
    }
    else if ((*newcon>-159) && (*newcon<-80))
    {
        vi[0] = -(*newcon) - 81;
        dUas[vi[0]] = -2;
        while (vi[0] >= 2)
        {
            vi[0] = vi[0] - 2;
            if ((dUas[vi[0]]>-1) && (dUas[vi[0]]<+1))
            {
                vi[0] = vi[0] + 2;
                break;
            }
        }
        if      (Uas[vi[0]] < -1)
        {
            vi[1] = -2;
        }
        else if (Uas[vi[0]] > +1)
        {
            vi[1] = +2;
        }
        else
        {
            vi[1] = 0;
        }
        vi[2] = vi[0];
        while (vi[2] < 78)
        {
            vi[2] = vi[2] + 2;
            if (vi[1] < -1)
            {
                if       (Uas[vi[2]] < -1)
                {
                    Uas[vi[2]] = -2;
                }
                else if (Uas[vi[2]] > +1)
                {
                    Uas[vi[2]] = -2;
                }
            }
            else
            {
                if       (Uas[vi[2]] < -1)
                {
                    Uas[vi[2]] = -2;
                    vi[1] = -2;
                }
                else if (Uas[vi[2]] > +1)
                {
                    Uas[vi[2]] = +2;
                    vi[1] = +2;
                }
            }
            if (vi[2] < 78)
            {
                if ((dUas[vi[2]]>-1) && (dUas[vi[2]]<+1))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if      ((vi[1]<-1) && (Uas[vi[0]]>-3))
        {
            Uas[vi[0]] = -4;
        }
        else if ((vi[1]>+1) && (Uas[vi[0]]<3))
        {
            Uas[vi[0]] = +4;
        }
        if ((vi[1]<-1) || (vi[1]>+1))
        {
            gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
        }
        else
        {
            gsavgxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2],&vd[0]);
        }
    }
    else if ((*newcon< 159) && (*newcon> 80))
    {
        vi[0] =  (*newcon) - 81;
        dUas[vi[0]] = +2;
        while (vi[0] >= 2)
        {
            vi[0] = vi[0] - 2;
            if ((dUas[vi[0]]>-1) && (dUas[vi[0]]<+1))
            {
                vi[0] = vi[0] + 2;
                break;
            }
        }
        if      (Uas[vi[0]] < -1)
        {
            vi[1] = -2;
        }
        else if (Uas[vi[0]] > +1)
        {
            vi[1] = +2;
        }
        else
        {
            vi[1] = 0;
        }
        vi[2] = vi[0];
        while (vi[2] < 78)
        {
            vi[2] = vi[2] + 2;
            if (vi[1] < -1)
            {
                if       (Uas[vi[2]] < -1)
                {
                    Uas[vi[2]] = -2;
                }
                else if (Uas[vi[2]] > +1)
                {
                    Uas[vi[2]] = -2;
                }
            }
            else
            {
                if       (Uas[vi[2]] < -1)
                {
                    Uas[vi[2]] = -2;
                    vi[1] = -2;
                }
                else if (Uas[vi[2]] > +1)
                {
                    Uas[vi[2]] = +2;
                    vi[1] = +2;
                }
            }
            if (vi[2] < 78)
            {
                if ((dUas[vi[2]]>-1) && (dUas[vi[2]]<+1))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if      ((vi[1]<-1) && (Uas[vi[0]]>-3))
        {
            Uas[vi[0]] = -4;
        }
        else if ((vi[1]>+1) && (Uas[vi[0]]<3))
        {
            Uas[vi[0]] = +4;
        }
        if ((vi[1]<-1) || (vi[1]>+1))
        {
            gszeroxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2]);
        }
        else
        {
            gsavgxfw(&Uas[0],&dUas[0],&vi[0],&x[0],&vi[2],&vd[0]);
        }
    }
    x[280] = 1;
    for (vi[0]=0; vi[0]<280; vi[0]++)
    {
        if      (x[vi[0]] >   x[280])
        {
            x[280] =   x[vi[0]];
        }
        else if (x[vi[0]] < - x[280])
        {
            x[280] = - x[vi[0]];
        }
    }
}
 
static void gsactrem(int8_t *Uas, int8_t *dUas, double const *lambda, uint8_t const *p, int16_t const *efp, uint8_t *brk, int64_t *vi)
{
    for (vi[0]=0; vi[0]<40; vi[0]++)
    {
        vi[2] = vi[0] * 7;
        for (vi[1]=0; vi[1]<p[vi[0]]; vi[1]++)
        {
            if (lambda[vi[2]+vi[1]] < -9.99999999999999980e-13)
            {
                *brk = 0;
                vi[3] = (vi[0]*2) + vi[1];
                if      (efp[vi[3]] > 0)
                {
                    vi[4] = efp[vi[3]] - 1;
                    if      ((Uas[vi[4]] < -5) || (Uas[vi[4]] > +5))
                    {
                        Uas[vi[4]] = 0;
                    }
                    else if ((Uas[vi[4]] < -3) || (Uas[vi[4]] > +3))
                    {
                        Uas[vi[4]] = 0;
                        if (vi[4] < 78)
                        {
                            if ((dUas[vi[4]]<-1) || (dUas[vi[4]]>+1))
                            {
                                vi[5] = 2;
                            }
                            else
                            {
                                vi[5] = 0;
                            }
                        }
                        else
                        {
                            vi[5] = 0;
                        }
                        while (vi[5] > 1)
                        {
                            vi[4] = vi[4] + 2;
                            Uas[vi[4]] = 0;
                            if (vi[4] < 78)
                            {
                                if ((dUas[vi[4]]<-1) || (dUas[vi[4]]>+1))
                                {
                                    vi[5] = 2;
                                }
                                else
                                {
                                    vi[5] = 0;
                                }
                            }
                            else
                            {
                                vi[5] = 0;
                            }
                        }
                    }
                }
                else if (efp[vi[3]] < 0)
                {
                    vi[4] = -efp[vi[3]] - 1;
                    dUas[vi[4]] = 0;
                    vi[5] = vi[4] + 2;
                    if      ((Uas[vi[5]]>+1) && (Uas[vi[5]]<+3))
                    {
                        Uas[vi[5]] = +6;
                    }
                    else if ((Uas[vi[5]]<-1) && (Uas[vi[5]]>-3))
                    {
                        Uas[vi[5]] = -6;
                    }
                    else
                    {
                        if (vi[5] < 78)
                        {
                            if ((dUas[vi[5]]<-1) || (dUas[vi[5]]>+1))
                            {
                                vi[6] = 2;
                            }
                            else
                            {
                                vi[6] = 0;
                            }
                        }
                        else
                        {
                            vi[6] = 0;
                        }
                        while (vi[6] > 1)
                        {
                            vi[5] = vi[5] + 2;
                            if      (Uas[vi[5]] > +1)
                            {
                                Uas[vi[5]] = +2;
                                Uas[vi[4]+2] = +4;
                                vi[6] = 0;
                            }
                            else if (Uas[vi[5]] < -1)
                            {
                                Uas[vi[5]] = -2;
                                Uas[vi[4]+2] = -4;
                                vi[6] = 0;
                            }
                            else
                            {
                                if (vi[5] < 78)
                                {
                                    if ((dUas[vi[5]]>-1) && (dUas[vi[5]]<+1))
                                    {
                                        vi[6] = 0;
                                    }
                                }
                                else
                                {
                                    vi[6] = 0;
                                }
                            }
                        }
                    }
                    if (vi[4] > 1)
                    {
                        if ((dUas[vi[4]-2]<-1) || (dUas[vi[4]-2]>+1))
                        {
                            vi[6] = 2;
                            if      (Uas[vi[4]] > +1)
                            {
                                Uas[vi[4]] = +2;
                                vi[5] = +2;
                            }
                            else if (Uas[vi[4]] < -1)
                            {
                                Uas[vi[4]] = -2;
                                vi[5] = -2;
                            }
                            else
                            {
                                vi[5] = 0;
                            }
                        }
                        else
                        {
                            if      ((Uas[vi[4]]<-3) && (Uas[vi[4]]>-5))
                            {
                                Uas[vi[4]] = 0;
                            }
                            else if ((Uas[vi[4]]>+3) && (Uas[vi[4]]<+5))
                            {
                                Uas[vi[4]] = 0;
                            }
                            vi[5] = 0;
                            vi[6] = 0;
                        }
                    }
                    else
                    {
                        if      ((Uas[vi[4]]<-3) && (Uas[vi[4]]>-5))
                        {
                            Uas[vi[4]] = 0;
                        }
                        else if ((Uas[vi[4]]>+3) && (Uas[vi[4]]<+5))
                        {
                            Uas[vi[4]] = 0;
                        }
                        vi[5] = 0;
                        vi[6] = 0;
                    }
                    while (vi[6] > 1)
                    {
                        vi[4] = vi[4] - 2;
                        if (vi[4] > 1)
                        {
                            if ((dUas[vi[4]-2]<-1) || (dUas[vi[4]-2]>+1))
                            {
                                if      (Uas[vi[4]] > +1)
                                {
                                    Uas[vi[4]] = +2;
                                    vi[5] = +2;
                                }
                                else if (Uas[vi[4]] < -1)
                                {
                                    Uas[vi[4]] = -2;
                                    vi[5] = -2;
                                }
                            }
                            else
                            {
                                vi[6] = 0;
                            }
                        }
                        else
                        {
                            vi[6] = 0;
                        }
                    }
                    if      ((Uas[vi[4]]>-5) && (Uas[vi[4]]<+5))
                    {
                        if      (vi[5] > +1)
                        {
                            Uas[vi[4]] = +4;
                        }
                        else if (vi[5] < -1)
                        {
                            Uas[vi[4]] = -4;
                        }
                        else
                        {
                            Uas[vi[4]] = 0;
                        }
                    }
                }
            }
        }
    }
}
void autompc_run(
    const double *t,
    const double *u0,
    const double *x0,
    const double *R,
    const double *Q,
    const double *Xcon,
    const double *Ucon,
    const double *Traj,
    double *ST,
    int8_t *outputmode,
    double *u,
    double *U,
    double *Ref,
    double *X)
{
    static double UconMod[8];
    static double A[1000];
    static double B[400];
    static int8_t E[160];
    static int8_t F[156];
    static uint8_t p[40];
    static int16_t efp[80];
    static int8_t Uas[80];
    static int8_t dUas[78];
    static double fx[280];
    static double lambda[280];
    static double x[281];
    static uint8_t drivmode;
    static int64_t k;
    static uint8_t j;
    static uint16_t itcount;
    static int16_t newcon;
    static uint8_t brk;
    static double alphasum;
    static double DJ[4];
    static int64_t vi[29];
    static double vd[6346];
    itcount = 0;
    brk = 0;
    drivmode = (uint8_t)(ST[87]+0.5);
    for (k=0; k<5; k++)
    {
        X[k] = ST[82+k];
    }
    if (drivmode > 0)
    {
        k = 1;
        gssim(&ST[0],X,&k,&drivmode,&vi[0],&vd[1]);
    }
    else
    {
        for (k=0; k<5; k++)
        {
            X[5+k] = X[k];
        }
    }
    vd[0] = 0;
    for (k=0; k<5; k++)
    {
        if ((x0[k]!=x0[k]) || ((x0[k]==(+1/vd[0])) || (x0[k]==(-1/vd[0]))))
        {
            X[k] = X[5+k];
        }
        else
        {
            X[k] = x0[k];
        }
    }
    for (k=0; k<5; k++)
    {
        ST[82+k] = X[k];
    }
        if (drivmode > 0)
        {
            k = 1;
            gssim(&ST[0],X,&k,&drivmode,&vi[0],&vd[0]);
            for (k=0; k<5; k++)
            {
                X[k] = X[5+k];
            }
        }
    if (*t >= 0)
    {
        vd[0] = 0;
        for (k=0; k<2; k++)
        {
            if ((u0[k]!=u0[k]) || ((u0[k]==(+1/vd[0])) || (u0[k]==(-1/vd[0]))))
            {
                if ((ST[2+k]!=ST[2+k]) || ((ST[2+k]==(+1/vd[0])) || (ST[2+k]==(-1/vd[0]))))
                {
                    U[k] = 0;
                }
                else
                {
                    U[k] = ST[2+k];
                }
            }
            else
            {
                U[k] = u0[k];
            }
        }
        vd[0] = 0;
        for (k=2; k<80; k++)
        {
            if ((ST[2+k]!=ST[2+k]) || ((ST[2+k]==(+1/vd[0])) || (ST[2+k]==(-1/vd[0]))))
            {
                U[k] = U[-2+k];
            }
            else
            {
                U[k] = ST[2+k];
            }
        }
        for (k=80; k<82; k++)
        {
            U[k] = U[-2+k];
        }
    }
    for (j=0; j<2; j++)
    {
        if (Ucon[4+j] == 0)
        {
            UconMod[4+j] = Ucon[4+j];
        }
        else
        {
            UconMod[j] = (Ucon[2+j]-Ucon[j]) / Ucon[4+j];
            vi[0] = (int64_t)(UconMod[j]*(1+10*macheps));
            vi[1] = (int64_t)(UconMod[j]*(1-10*macheps));
            if (vi[0] == vi[1])
            {
                UconMod[4+j] = Ucon[4+j];
            }
            else
            {
                if (Ucon[4+j] < 0)
                {
                    UconMod[4+j] = (1-10*macheps)*Ucon[4+j];
                }
                else
                {
                    UconMod[4+j] = (1+10*macheps)*Ucon[4+j];
                }
            }
        }
        if (Ucon[6+j] == 0)
        {
            UconMod[6+j] = Ucon[6+j];
        }
        else
        {
            UconMod[j] = (Ucon[2+j]-Ucon[j]) / Ucon[6+j];
            vi[0] = (int64_t)(UconMod[j]*(1+10*macheps));
            vi[1] = (int64_t)(UconMod[j]*(1-10*macheps));
            if (vi[0] == vi[1])
            {
                UconMod[6+j] = Ucon[6+j];
            }
            else
            {
                if (Ucon[6+j] > 0)
                {
                    UconMod[6+j] = (1-10*macheps)*Ucon[6+j];
                }
                else
                {
                    UconMod[6+j] = (1+10*macheps)*Ucon[6+j];
                }
            }
        }
        UconMod[       j] = Ucon[       j];
        UconMod[2+j] = Ucon[2+j];
    }
    for (k=0; k<41; k++)
    {
        vi[0] = k * 2;
        for (j=0; j<2; j++)
        {
            if (U[vi[0]+j] < UconMod[  j])
            {
                U[vi[0]+j] = UconMod[  j];
            }
            if (U[vi[0]+j] > UconMod[2+j])
            {
                U[vi[0]+j] = UconMod[2+j];
            }
        }
    }
    for (k=0; k<40; k++)
    {
        vi[0] = (k+1) * 2;
        for (j=0; j<2; j++)
        {
            if (U[vi[0]+j] < U[vi[0]-2+j] + UconMod[4+j])
            {
                U[vi[0]+j] = U[vi[0]-2+j] + UconMod[4+j];
            }
            if (U[vi[0]+j] > U[vi[0]-2+j] + UconMod[6+j])
            {
                U[vi[0]+j] = U[vi[0]-2+j] + UconMod[6+j];
            }
        }
    }
    gsrefgen(t,X,Traj,&UconMod[0],&Ucon[2],&drivmode,Ref,&vi[0],&vd[0]);
    if (drivmode == 0)
    {
        for (k=0; k<40; k++)
        {
            vi[0] = k * 2;
            vi[1] = (k+1) * 2;
            if (Ref[4] < 0)
            {
                U[vi[1]  ] = Ref[4];
            }
            else
            {
                U[vi[1]  ] = 0;
            }
            U[vi[1]+1] = (Ref[5]-X[4]) / 1.00000000000000006e-01;
            for (j=2; j<2; j++)
            {
                U[vi[1]+j] = 0;
            }
            for (j=0; j<2; j++)
            {
                if      (U[vi[1]+j] < U[vi[0]+j] + UconMod[4+j])
                {
                    U[vi[1]+j] = U[vi[0]+j] + UconMod[4+j];
                }
                else if (U[vi[1]+j] > U[vi[0]+j] + UconMod[6+j])
                {
                    U[vi[1]+j] = U[vi[0]+j] + UconMod[6+j];
                }
                if      (U[vi[1]+j] < UconMod[j])
                {
                    U[vi[1]+j] = UconMod[j];
                }
                else if (U[vi[1]+j] > UconMod[2+j])
                {
                    U[vi[1]+j] = UconMod[2+j];
                }
            }
        }
    }
    else
    {
    gsactset(U,&Uas[0],&dUas[0],&UconMod[0],&vi[0],&vd[0]);
    if (*t >= 0)
    {
        do
        {
        itcount++;
        alphasum = 0;
        k = 40;
        gssim(U,X,&k,&drivmode,&vi[0],&vd[0]);
        gslin(U,X,&A[0],&B[0],&drivmode,&vi[0],&vd[0]);
        gseqcon(&Uas[0],&dUas[0],&E[0],&F[0],&p[0],&efp[0],&vi[0]);
        gscostf(U,X,Ref,R,Q,&Xcon[0],&Xcon[1],&fx[0],&vi[0],&vd[0]);
        gskkt(R,Q,Ref,&fx[0],&A[0],&B[0],&E[0],&F[0],&p[0],&vd[4],&vd[284],&vd[3315],&lambda[0],&x[0],&vi[0],&vd[0]);
        gsmodx(&Uas[0],&dUas[0],&x[0],&vi[0]);
        gsline(U,X,&Uas[0],&dUas[0],&UconMod[0],Ref,&drivmode,R,Q,&Xcon[0],&Xcon[1],&x[0],&DJ[0],&newcon,&alphasum,&vi[0],&vd[0]);
        j = 0;
        if (alphasum >= 9.99900000000000011e-01)
        {
            j = 20;
            brk = 0;
        }
        while ((newcon!=0) && (j<20))
        {
            j++;
            gsactadd(&Uas[0],&dUas[0],&x[0],&newcon,&vi[0],&vd[0]);
            gsline(U,X,&Uas[0],&dUas[0],&UconMod[0],Ref,&drivmode,R,Q,&Xcon[0],&Xcon[1],&x[0],&DJ[0],&newcon,&alphasum,&vi[0],&vd[0]);
            if (alphasum >= 1)
            {
                j = 20;
            }
        }
        if      (brk < 1)
        {
            brk = 2;
        }
        else if (brk < 3)
        {
            brk = 4;
        }
        gsactrem(&Uas[0],&dUas[0],&lambda[0],&p[0],&efp[0],&brk,&vi[0]);
        if (itcount > 9)
        {
            brk = 4;
        }
    } while (brk < 3);
    }
    }
    for (k=0; k<2; k++)
    {
        if ((U[2+k]!=U[2+k]) || ((U[2+k]==(+1/vd[0])) || (U[2+k]==(-1/vd[0]))))
        {
            U[2+k] = U[k];
            u[k] = U[k];
        }
        else
        {
            u[k] = U[2+k];
        }
    }
    for (k=0; k<41; k++)
    {
        for (j=0; j<2; j++)
        {
            ST[k*2+j] = U[k*2+j];
        }
    }
    if (drivmode > 0)
    {
        k = 40;
        gssim(U,X,&k,&drivmode,&vi[0],&vd[0]);
    }
    ST[87] = (double)(drivmode);
    *outputmode = (int8_t)(drivmode);
}

