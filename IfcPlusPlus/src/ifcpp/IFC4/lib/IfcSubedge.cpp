/* Code generated by IfcQuery EXPRESS generator, www.ifcquery.com */
#include <sstream>
#include <limits>

#include "ifcpp/model/IfcPPException.h"
#include "ifcpp/model/IfcPPAttributeObject.h"
#include "ifcpp/model/IfcPPGuid.h"
#include "ifcpp/reader/ReaderUtil.h"
#include "ifcpp/writer/WriterUtil.h"
#include "ifcpp/IFC4/include/IfcEdge.h"
#include "ifcpp/IFC4/include/IfcPresentationLayerAssignment.h"
#include "ifcpp/IFC4/include/IfcStyledItem.h"
#include "ifcpp/IFC4/include/IfcSubedge.h"
#include "ifcpp/IFC4/include/IfcVertex.h"

// ENTITY IfcSubedge 
IfcSubedge::IfcSubedge() {}
IfcSubedge::IfcSubedge( int id ) { m_id = id; }
IfcSubedge::~IfcSubedge() {}
shared_ptr<IfcPPObject> IfcSubedge::getDeepCopy( IfcPPCopyOptions& options )
{
	shared_ptr<IfcSubedge> copy_self( new IfcSubedge() );
	if( m_EdgeStart ) { copy_self->m_EdgeStart = dynamic_pointer_cast<IfcVertex>( m_EdgeStart->getDeepCopy(options) ); }
	if( m_EdgeEnd ) { copy_self->m_EdgeEnd = dynamic_pointer_cast<IfcVertex>( m_EdgeEnd->getDeepCopy(options) ); }
	if( m_ParentEdge ) { copy_self->m_ParentEdge = dynamic_pointer_cast<IfcEdge>( m_ParentEdge->getDeepCopy(options) ); }
	return copy_self;
}
void IfcSubedge::getStepLine( std::stringstream& stream ) const
{
	stream << "#" << m_id << "= IFCSUBEDGE" << "(";
	if( m_EdgeStart ) { stream << "#" << m_EdgeStart->m_id; } else { stream << "*"; }
	stream << ",";
	if( m_EdgeEnd ) { stream << "#" << m_EdgeEnd->m_id; } else { stream << "*"; }
	stream << ",";
	if( m_ParentEdge ) { stream << "#" << m_ParentEdge->m_id; } else { stream << "$"; }
	stream << ");";
}
void IfcSubedge::getStepParameter( std::stringstream& stream, bool ) const { stream << "#" << m_id; }
void IfcSubedge::readStepArguments( const std::vector<std::wstring>& args, const map_t<int,shared_ptr<IfcPPEntity> >& map )
{
	const size_t num_args = args.size();
	if( num_args != 3 ){ std::stringstream err; err << "Wrong parameter count for entity IfcSubedge, expecting 3, having " << num_args << ". Entity ID: " << m_id << std::endl; throw IfcPPException( err.str().c_str() ); }
	readEntityReference( args[0], m_EdgeStart, map );
	readEntityReference( args[1], m_EdgeEnd, map );
	readEntityReference( args[2], m_ParentEdge, map );
}
void IfcSubedge::getAttributes( std::vector<std::pair<std::string, shared_ptr<IfcPPObject> > >& vec_attributes )
{
	IfcEdge::getAttributes( vec_attributes );
	vec_attributes.push_back( std::make_pair( "ParentEdge", m_ParentEdge ) );
}
void IfcSubedge::getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<IfcPPObject> > >& vec_attributes_inverse )
{
	IfcEdge::getAttributesInverse( vec_attributes_inverse );
}
void IfcSubedge::setInverseCounterparts( shared_ptr<IfcPPEntity> ptr_self_entity )
{
	IfcEdge::setInverseCounterparts( ptr_self_entity );
}
void IfcSubedge::unlinkFromInverseCounterparts()
{
	IfcEdge::unlinkFromInverseCounterparts();
}