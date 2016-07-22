﻿#ifndef ENVIRONMENT_INCLUDED
#define ENVIRONMENT_INCLUDED

#include <particle_simulator.hpp>

namespace OpenMps
{
	// MPS計算用の計算空間固有パラメータ
	class Environment final
	{
	private:
		// 現在時刻
		double t;

		// 時間刻み
		double dt;

		// 基準粒子数密度
		double n0;

		// 拡散モデル定数
		double lambda;

		// 最大時間刻み
		const double maxDt;

		// 1ステップの最大移動距離
		const double maxDx;

	public:

		// 初期粒子間距離
		const double L_0;

		// 影響半径
		const double R_e;

		// 重力加速度
		const PS::F64vec G;

		// 密度
		const double Rho;

		// 動粘性係数
		const double Nu;

		// 自由表面を判定する係数
		const double SurfaceRatio;

#ifdef MODIFY_TOO_NEAR
		// 過剰接近粒子と判定される距離
		const double TooNearLength;

		// 過剰接近粒子から受ける修正量の係数
		const double TooNearCoefficient;
#endif

#ifdef PRESSURE_EXPLICIT
		// 音速
		const double C;
#endif

		// 計算空間の最小座標
		const PS::F64vec MinX;

		// 計算空間の最大座標
		const PS::F64vec MaxX;

		// 近傍粒子として保持する距離
		const double NeighborLength;

		// @param maxDt 最大時間刻み（出力時間刻み以下など）
		// @param courant クーラン数
		// @param tooNearRatio 過剰接近粒子と判定される距離（初期粒子間距離との比）
		// @param tooNearCoeffcient 過剰接近粒子から受ける修正量の係数
		// @param g 重力加速度
		// @param rho 密度
		// @param nu 動粘性係数
		// @param r_eByl_0 影響半径と初期粒子間距離の比
		// @param surfaceRatio 自由表面判定の係数
		// @param c 音速
		// @param l_0 初期粒子間距離
		// @param minX 計算空間の最小X座標
		// @param minZ 計算空間の最小Z座標
		// @param maxX 計算空間の最大X座標
		// @param maxZ 計算空間の最大Z座標
		Environment(
			const double maxDt,
			const double courant,
#ifdef MODIFY_TOO_NEAR
			const double tooNearRatio,
			const double tooNearCoefficient,
#endif
			const double g,
			const double rho,
			const double nu,
			const double surfaceRatio,
			const double r_eByl_0,
#ifdef PRESSURE_EXPLICIT
			const double c,
#endif
			const double l_0,
			const double minX, const double minZ,
			const double maxX, const double maxZ)
			:t(0), dt(0), L_0(l_0), G(0, -g), Rho(rho), Nu(nu), maxDx(courant*l_0), R_e(r_eByl_0 * l_0), SurfaceRatio(surfaceRatio),

#ifdef MODIFY_TOO_NEAR
			TooNearLength(tooNearRatio*l_0), TooNearCoefficient(tooNearCoefficient),
#endif
#ifdef PRESSURE_EXPLICIT
			C(c),

			// 最大時間刻みは、dx < c dt （音速での時間刻み制限）と、指定された引数のうち小さい方
			maxDt(std::min(maxDt, (courant*l_0) / c)),
#else
			// 最大時間刻みは、dx < 1/2 g dt^2 （重力による等加速度運動での時間刻み制限）と、指定された引数のうち小さい方
			maxDt(std::min(maxDt, std::sqrt(2 * (courant*l_0) / g))),
#endif
			MinX(minX, minZ), MaxX(maxX, maxZ),
			NeighborLength(r_eByl_0 * l_0 * (1 + courant*2)) // 計算の安定化のためクーラン数の2倍の距離までを近傍粒子として保持する
		{
			// 基準粒子数密度とλの計算
			const auto range = static_cast<int>(std::ceil(r_eByl_0));
			n0 = 0;
			lambda = 0;
			for (auto i = -range; i < range; i++)
			{
				for (auto j = -range; j < range; j++)
				{
					// 自分以外との
					if (!((i == 0) && (j == 0)))
					{
						// 相対位置を計算
						const auto x = PS::F64vec(i*l_0, j*l_0);

						// 重み関数を計算
						const auto r = std::sqrt(x*x);
						const auto w = Particle::W(r, R_e);

						// 基準粒子数密度に足す
						n0 += w;

						// λに足す
						lambda += r*r * w;
					}
				}
			}

			// λの最終計算
			// λ = (Σr^2 w)/(Σw)
			lambda /= n0;
		}

		// する
		void SetDt(const double maxU)
		{
			dt = (maxU == 0 ? maxDt : std::min(maxDx/maxU, maxDt));
		}
		// CFL条件より時間刻みを決定する

		// 時刻を進める
		void SetNextT()
		{
			t += dt;
		}

		double T() const
		{
			return t;
		}

		double Dt() const
		{
			return dt;
		}

		double N0() const
		{
			return n0;
		}

		double Lambda() const
		{
			return lambda;
		}

		// 代入演算子
		// @param src 代入元
		Environment& operator=(const Environment& src)
		{	
			this->t = src.t;
			this->dt = src.dt;
			this->n0 = src.n0;
			this->lambda = src.lambda;
			const_cast<double&>(this->maxDt) = src.maxDt;
			const_cast<double&>(this->maxDx) = src.maxDx;
			const_cast<double&>(this->R_e) = src.R_e;
			const_cast<std::remove_const_t<decltype(this->G)>&>(this->G) = src.G;
			const_cast<double&>(this->Rho) = src.Rho;
			const_cast<double&>(this->Nu) = src.Nu;
			const_cast<double&>(this->SurfaceRatio) = src.SurfaceRatio;
#ifdef MODIFY_TOO_NEAR
			const_cast<double&>(TooNearLength) = src.TooNearLength;
			const_cast<double&>(TooNearCoefficient) = src.TooNearCoefficient;
#endif
#ifdef PRESSURE_EXPLICIT
			const_cast<double&>(C) = src.C;
#endif
			return *this;
		}
	};
}
#endif
