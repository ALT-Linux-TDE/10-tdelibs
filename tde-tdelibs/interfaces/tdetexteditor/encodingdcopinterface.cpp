#include "encodingdcopinterface.h"
#include "encodinginterface.h"

#include <dcopclient.h>
using namespace KTextEditor;

EncodingDCOPInterface::EncodingDCOPInterface( EncodingInterface *Parent, const char *name)
	: DCOPObject(name)
{
	m_parent = Parent;
}

EncodingDCOPInterface::~EncodingDCOPInterface()
{

 }
uint EncodingDCOPInterface::encodingInterfaceNumber ()
{
	return m_parent->encodingInterfaceNumber ();
}
void EncodingDCOPInterface::setEncoding (TQString e) 
{
	m_parent->setEncoding (e); 
}
TQString EncodingDCOPInterface::encoding()
{
	return m_parent->encoding();
}
