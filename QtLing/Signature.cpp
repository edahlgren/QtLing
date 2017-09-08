#include "Signature.h"
#include "Suffix.h"

CSignature::CSignature(CStringSurrogate ssWord) : CParse(ssWord), m_Signature(ssWord.GetKey()), m_Affixes() {}

CSignature::CSignature(CSignature& signature) {
    m_Signature = signature.GetSignature();
    m_Affixes = signature.GetList();
}

CSignature::CSignature(QString  qstring) {
    m_Signature = qstring;
}



void CSignature::add_stem_pointer(CStem* pStem)
{
    m_Stems.append(pStem);
    
}

QString CSignature::display()
{
    return m_Signature;
}

QString CSignature::display_stems()
{
    QString outstring;
    CStem *qStem;
    foreach (qStem, m_Stems){
        outstring += " " + qStem->display();
    }
    return outstring;
}
int CSignature::get_robustness()
{
    int robustness = 0;
    int stem_letters = 0;
    int suffix_letters = 0;
    foreach (const CSuffix* suffix, m_Affixes){
        if (suffix->get_key() == "NULL"){
            suffix_letters += 1;
        } else{
            suffix_letters += suffix->get_key().length();
        }
    }
    foreach (const CStem* stem, m_Stems){
        stem_letters += stem->get_key().length();
    }
    robustness = stem_letters + suffix_letters;
    return robustness;
}
