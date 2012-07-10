#ifndef CustomValidator_h
#define CustomValidator_h

#include <QValidator>

class OddValidator : public QValidator
{
  QValidator::State validate(QString & input, int & pos) const;

  void fixup(QString & input) const;
};

#endif
