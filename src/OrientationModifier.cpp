

#include "stdafx.h"
#include "OrientationModifier.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyCheck.h"
#include "HeeksConfig.h"
#include <BRepAdaptor_Curve.hxx>

void COrientationModifierParams::set_initial_values()
{
	HeeksConfig config;

	config.Read(_T("OrientationModifier_m_spacing"), (int *) &m_spacing, int(eNormalSpacing));
	config.Read(_T("OrientationModifier_number_of_rotations"), (int *) &m_number_of_rotations, 0);
	config.Read(_T("OrientationModifier_sketch_rotates_text"), &m_sketch_rotates_text, false);
	config.Read(_T("OrientationModifier_justification"), (int *) &m_justification, int(eLeftJustified));
}

void COrientationModifierParams::write_values_to_config()
{
	// We always want to store the parameters in mm and convert them back later on.

	HeeksConfig config;

	// These values are in mm.
	config.Write(_T("OrientationModifier_m_spacing"), m_spacing);
	config.Write(_T("OrientationModifier_number_of_rotations"), m_number_of_rotations);
	config.Write(_T("OrientationModifier_sketch_rotates_text"), m_sketch_rotates_text);
	config.Write(_T("OrientationModifier_justification"), m_justification);
}

static void on_set_justification(int zero_based_choice, HeeksObj* object)
{
	((COrientationModifier*)object)->m_params.m_justification = COrientationModifierParams::eJustification_t(zero_based_choice);
	((COrientationModifier*)object)->m_params.write_values_to_config();
}

static void on_set_spacing(int zero_based_choice, HeeksObj* object)
{
	((COrientationModifier*)object)->m_params.m_spacing = COrientationModifierParams::eSpacing_t(zero_based_choice);
	((COrientationModifier*)object)->m_params.write_values_to_config();
}

static void on_set_number_of_rotations(int number_of_rotations, HeeksObj* object)
{
	((COrientationModifier*)object)->m_params.m_number_of_rotations = number_of_rotations;
	((COrientationModifier*)object)->m_params.write_values_to_config();
}

static void on_set_sketch_rotates_text(bool value, HeeksObj* object)
{
	((COrientationModifier*)object)->m_params.m_sketch_rotates_text = value;
	((COrientationModifier*)object)->m_params.write_values_to_config();
}

void COrientationModifierParams::GetProperties(COrientationModifier * parent, std::list<Property *> *list)
{

	{
		int choice = int(m_spacing);
		std::list< wxString > choices;
		choices.push_back( wxString(_("Normally spaced")) );

		list->push_back(new PropertyChoice(_("Spacing"), choices, choice, parent, on_set_spacing));
	}

	list->push_back(new PropertyInt(_("Number of Rotations (negative for reverse direction)"), m_number_of_rotations, parent, on_set_number_of_rotations));
	list->push_back(new PropertyCheck(_("Sketch rotates text"), m_sketch_rotates_text, parent,  on_set_sketch_rotates_text));

	{
		int choice = int(m_justification);
		std::list< wxString > choices;
		choices.push_back( wxString(_("Left")) );
		choices.push_back( wxString(_("Centre")) );
		choices.push_back( wxString(_("Right")) );

		list->push_back(new PropertyChoice(_("Justification"), choices, choice, parent, on_set_justification));
	}
}

void COrientationModifierParams::WriteXMLAttributes(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "params" );
	root->LinkEndChild( element );

	element->SetAttribute("m_spacing", int(m_spacing));
	element->SetAttribute("m_number_of_rotations", int(m_number_of_rotations));
	element->SetAttribute("m_sketch_rotates_text", m_sketch_rotates_text);
	element->SetAttribute("m_justification", int(m_justification));
}

void COrientationModifierParams::ReadParametersFromXMLElement(TiXmlElement* pElem)
{
	if (pElem->Attribute("m_spacing")) pElem->Attribute("m_spacing", (int *) &m_spacing);
	if (pElem->Attribute("m_number_of_rotations")) pElem->Attribute("m_number_of_rotations", (int *) &m_number_of_rotations);

	if (pElem->Attribute("m_sketch_rotates_text"))
	{
	    int flag = 0;
	    pElem->Attribute("m_sketch_rotates_text", (int *) &flag);
	    m_sketch_rotates_text = (flag != 0);
	}
	else
	{
	    m_sketch_rotates_text = false;
	}

	if (pElem->Attribute("m_justification")) pElem->Attribute("m_justification", (int *) &m_justification);
}


COrientationModifier::COrientationModifier( const COrientationModifier & rhs ) : ObjList(rhs)
{
	m_params = rhs.m_params;
}

COrientationModifier & COrientationModifier::operator= ( const COrientationModifier & rhs )
{
	if (this != &rhs)
	{
	    m_params = rhs.m_params;
		ObjList::operator=(rhs);
	}

	return(*this);
}

const wxBitmap &COrientationModifier::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/text.png")));
	return *icon;
}

void COrientationModifier::glCommands(bool select, bool marked, bool no_color)
{
	ObjList::glCommands( select, marked, no_color );
}

HeeksObj *COrientationModifier::MakeACopy(void)const
{
	COrientationModifier *new_object = new COrientationModifier(*this);
	return(new_object);
}

void COrientationModifier::CopyFrom(const HeeksObj* object)
{
	if (object->GetType() == OrientationModifierType)
	{
		*this = *((COrientationModifier *) object);
	}
}


bool COrientationModifier::CanAddTo(HeeksObj* owner)
{
	return((owner != NULL) && (owner->GetType() == TextType));
}

bool COrientationModifier::CanAdd(HeeksObj* object)
{
    if (object == NULL) return(false);

	// We only want a single sketch.  Make sure we don't already have one.
	if (GetNumChildren() > 0)
	{
	    // Don't popup a warning as this code gets called by the Undo engine
	    // when the file is initially read in as well.

	    /*
		wxString message;
		message << _("Only a single sketch is supported");
		wxMessageBox(message);
        */

		return(false);
	}

	switch (object->GetType())
	{
		case SketchType:
			return(true);

		default:
			return(false);
	} // End switch
}

void COrientationModifier::GetProperties(std::list<Property *> *list)
{
	m_params.GetProperties( this, list );
	ObjList::GetProperties(list);
}


void COrientationModifier::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	ObjList::GetTools( t_list, p );
}



void COrientationModifier::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "OrientationModifier" );
	root->LinkEndChild( element );

	m_params.WriteXMLAttributes(element);

	WriteBaseXML(element);
}

// static member function
HeeksObj* COrientationModifier::ReadFromXMLElement(TiXmlElement* element)
{
	COrientationModifier* new_object = new COrientationModifier();

	std::list<TiXmlElement *> elements_to_remove;

	// read point and circle ids
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string name(pElem->Value());
		if(name == "params"){
			new_object->m_params.ReadParametersFromXMLElement(pElem);
			elements_to_remove.push_back(pElem);
		}
	}

	for (std::list<TiXmlElement*>::iterator itElem = elements_to_remove.begin(); itElem != elements_to_remove.end(); itElem++)
	{
		element->RemoveChild(*itElem);
	}

	new_object->ReadBaseXML(element);

	return new_object;
}


void COrientationModifier::ReloadPointers()
{
	ObjList::ReloadPointers();
}



gp_Pnt & COrientationModifier::Transform(gp_Trsf existing_transformation, const double _distance, gp_Pnt & point, const float width )
{
	double tolerance = wxGetApp().m_geom_tol;

    if (GetNumChildren() == 0)
	{
		// No children so no modification of position.
		return(point);
	}

	gp_Pnt origin(0.0, 0.0, 0.0);
	gp_Trsf move_to_origin;
	move_to_origin.SetTranslation(origin, gp_Pnt(-1.0 * _distance, 0.0, 0.0));
	point.Transform(move_to_origin);

	double distance(_distance);
	if (distance < 0) distance *= -1.0;

	// The distance in font coordinates is smaller than that used to place the characters.
	// Scale this distance up by the HText scale factor being applied to the font graphics.
	// distance *= existing_transformation.ScaleFactor();

	std::list<TopoDS_Shape> wires;
	if (! ::ConvertSketchToFaceOrWire( GetFirstChild(), wires, false))
    {
		return(point);
    } // End if - then

	typedef std::list<std::pair<TopoDS_Edge, double> > Edges_t;
	Edges_t edges;

	double total_edge_length = 0.0;
	for (std::list<TopoDS_Shape>::iterator itWire = wires.begin(); itWire != wires.end(); itWire++)
	{
		TopoDS_Wire wire(TopoDS::Wire(*itWire));

		for(BRepTools_WireExplorer expEdge(TopoDS::Wire(wire)); expEdge.More(); expEdge.Next())
		{
			TopoDS_Edge edge(TopoDS_Edge(expEdge.Current()));

			BRepAdaptor_Curve curve(edge);
			double edge_length = GCPnts_AbscissaPoint::Length(curve);
			edges.push_back( std::make_pair(edge,edge_length) );
			total_edge_length += edge_length;
		} // End for
	} // End for

	// Now adjust the distance based on a combination of the justification and how far through
	// the text string we are.
	double distance_remaining = distance;

	switch (m_params.m_justification)
	{
	case COrientationModifierParams::eLeftJustified:
		distance_remaining = distance;	// No special adjustment required.
		break;

	case COrientationModifierParams::eRightJustified:
		distance_remaining = total_edge_length - width + distance;
		break;

	case COrientationModifierParams::eCentreJustified:
		distance_remaining = (total_edge_length / 2.0) - (width / 2.0) + distance;
		break;
	} // End switch


    while (distance_remaining > tolerance)
    {
        for (Edges_t::iterator itEdge = edges.begin(); itEdge != edges.end(); itEdge++)
        {

			double edge_length = itEdge->second;
			if (edge_length < distance_remaining)
			{
				distance_remaining -= edge_length;
			}
			else
			{
				// The point we're after is along this edge somewhere.  Find the point and
				// the first derivative at that point.
				BRepAdaptor_Curve curve(itEdge->first);
				gp_Pnt p;
				gp_Vec vec;
				double proportion = distance_remaining / edge_length;
				Standard_Real U = ((curve.LastParameter() - curve.FirstParameter()) * proportion) + curve.FirstParameter();
				curve.D1(U, p, vec);

				double angle = 0.0;
				if (m_params.m_sketch_rotates_text)
				{
				    angle = gp_Vec(0,1,0).Angle(vec);
				}

				if (m_params.m_number_of_rotations > 0)
				{
				    for (int i=0; i<m_params.m_number_of_rotations; i++)
				    {
				        angle += (PI / 2.0);
				    }
				}
				else
				{
				    for (int i=m_params.m_number_of_rotations; i<0; i++)
				    {
				        angle -= (PI / 2.0);
				    }
				}

				angle *= -1.0;
				gp_Trsf transformation;
				transformation.SetRotation( gp_Ax1(origin, gp_Vec(0,0,-1)), angle );
				point.Transform( transformation );

				gp_Trsf around;
				around.SetTranslation(origin, p);
				point.Transform(around);

				point.Transform(existing_transformation);   // Restore the original HText transformation.

				return(point);
			}
		} // End for
	} // End while

	return(point);
}




