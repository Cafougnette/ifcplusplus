/* -*-c++-*- IfcPlusPlus - www.ifcplusplus.com  - Copyright (C) 2011 Fabian Gerold
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#pragma once

#include <ifcpp/model/shared_ptr.h>
#include <ifcpp/model/StatusCallback.h>
#include "IncludeCarveHeaders.h"
#include "GeometryInputData.h"

class GeometrySettings;
class UnitConverter;
class CurveConverter;
class SplineConverter;
class Sweeper;
class IfcFace;
class IfcRationalBSplineSurfaceWithKnots;

class SurfaceProxy
{
public:
	virtual void computePointOnSurface(const carve::geom::vector<3>& point_in, carve::geom::vector<3>& point_out) = 0;
};

class SurfaceProxyLinear : public SurfaceProxy
{
public:
	virtual void computePointOnSurface(const carve::geom::vector<3>& point_in, carve::geom::vector<3>& point_out);
	carve::math::Matrix m_surface_matrix;
};

class FaceConverter : public StatusCallback
{
public:
	enum ShellType { SHELL_TYPE_UNKONWN, OPEN_SHELL, CLOSED_SHELL };
	FaceConverter( shared_ptr<GeometrySettings>& gs, shared_ptr<UnitConverter>& uc, shared_ptr<CurveConverter>& cc, shared_ptr<SplineConverter>& sc, shared_ptr<Sweeper>& sw );
	~FaceConverter();

	void convertIfcFaceList(		const std::vector<shared_ptr<IfcFace> >& faces,		shared_ptr<ItemShapeInputData> item_data, ShellType st );
	void convertIfcSurface(			const shared_ptr<IfcSurface>& surface,				shared_ptr<carve::input::PolylineSetData>& polyline_data, shared_ptr<SurfaceProxy>& surface_proxy );

	shared_ptr<GeometrySettings>	m_geom_settings;
	shared_ptr<UnitConverter>		m_unit_converter;
	shared_ptr<CurveConverter>		m_curve_converter;
	shared_ptr<SplineConverter>		m_spline_converter;
	shared_ptr<Sweeper>				m_sweeper;
};
