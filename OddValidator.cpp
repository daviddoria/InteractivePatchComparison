#include "OddValidator.h"

QValidator::State OddValidator::validate(QString & input, int & pos) const
{
  if(input.size() == 0)
  {
    return QValidator::Intermediate;
  }
  
  if(input.toUInt() % 2 == 0)
  {
    return QValidator::Invalid;
  }

  return QValidator::Acceptable;
}

void OddValidator::fixup(QString & input) const
{

}
