#include <QPair>
#include <QList>
#include <QMap>
#include <QMapIterator>
#include <QMultiMap>
#include <QSet>
#include <QtDebug>
#include <QProgressBar>
#include <QApplication>
#include <algorithm>
#include <QChar>
#include "Lexicon.h"

#include "SignatureCollection.h"
#include "StemCollection.h"
#include "SuffixCollection.h"
#include "WordCollection.h"
#include "Word.h"

CLexicon::CLexicon(bool suffix_flag) : m_Words(new CWordCollection), m_Suffixes(new CSuffixCollection), m_Prefixes(new CPrefixCollection),
    m_Signatures(new CSignatureCollection)
{   m_Stems                 = new CStemCollection();
    m_Compounds             = new CWordCollection();
    m_Parses                = new QList<QPair<QString,QString>>();
    m_Protostems            = QMap<QString, int>();
    m_ParaSignatures    =  new CSignatureCollection();
    m_ParaSuffixes          = new CSuffixCollection();
    m_ResidualStems         = new CStemCollection();
    m_PrefixSignatures      = new CSignatureCollection();
    m_SuffixesFlag = true;
}

QListIterator<simple_sig_tree_edge*> * CLexicon::get_sig_tree_edge_list_iter()
{
    QListIterator<simple_sig_tree_edge*> * iter = new QListIterator<simple_sig_tree_edge*>(m_SigTreeEdgeList);
    return iter;
}

QMapIterator<QString, sig_tree_edge*> * CLexicon::get_sig_tree_edge_map_iter()
{
    QMapIterator<QString, sig_tree_edge*> * iter = new QMapIterator<QString, sig_tree_edge*>(m_SigTreeEdgeMap);
    return iter;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//linguistic methods


void CLexicon::Crab_1()
{
    FindProtostems();
    CreateStemAffixPairs();

    AssignSuffixesToStems();

    collect_parasuffixes();
    m_SuffixesFlag?
        m_Signatures->compute_containment_list():
        m_PrefixSignatures->compute_containment_list();

    qDebug() << "finished making signatures.";
 }
void CLexicon::Crab_2()
{
    ReSignaturizeWithKnownAffixes();
    FindGoodSignaturesInsideParaSignatures();
    compute_sig_tree_edges();
    compute_sig_tree_edge_map();
    qDebug() << "finished crab 2.";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * This is the first of the three initial parts of finding signatures.
 * This makes a cut at every point in a word where the successor frequency
 * is greater than 1.
 */
void CLexicon::FindProtostems()
{   word_t          this_word, previous_word;
    QStringList *   Words =  GetWordCollection()->GetSortedStringArray();
    bool            StartFlag = true;
    bool            DifferenceFoundFlag = false;
    stem_t          stem;
    int             this_word_length(0), previous_word_length(0);
    for (int wordno=0; wordno<Words->size(); wordno ++){
        if (m_SuffixesFlag){
            this_word = Words->at(wordno);
        } else{
            this_word = get_words()->get_reverse_sort_list()->at(wordno);
        }
        this_word_length = this_word.length();
        if (StartFlag){
            StartFlag = false;
            previous_word = this_word;
            previous_word_length = this_word_length;
            continue;
        }
        DifferenceFoundFlag = false;
        if (m_SuffixesFlag){
            int end = qMin(this_word_length, previous_word_length);
            for (int i=0; i <end; i++){
                if (previous_word[i] != this_word[i]){
                    stem = previous_word.left(i);
                    DifferenceFoundFlag = true;
                    if (!m_Protostems.contains(stem))                {
                        m_Protostems[stem] = 1;
                    }
                    break;
                }
            }
        }  // end of suffix case.
        else
        {       // -->   Prefix case   <-- //
            this_word_length = this_word.length();
            previous_word_length = previous_word.length();
            int end = qMin(this_word_length, previous_word_length);
            for (int i=0; i <end; i++){
                if (previous_word[previous_word_length - i] != this_word[ this_word_length - i]){
                    stem = previous_word.right(i-1);
                    DifferenceFoundFlag = true;

                    if (!m_Protostems.contains(stem))                {
                        m_Protostems[stem] = 1;
                    }
                    break;
                }
            }
        }
        if (DifferenceFoundFlag == true){
            previous_word = this_word;
            continue;
        }
        else {
            if (previous_word.length() < this_word.length()) {
                m_Protostems[previous_word] = 1;
            }
        }
        previous_word = this_word;
        previous_word_length = this_word_length;
    }
    qDebug() << "Finished finding protostems.";
    return;
}

/*!
 * This is the second of the three initial parts of finding signatures.
 * This creates stem/affix pairs, which are put in a long list of "Parses".
 */
void CLexicon::CreateStemAffixPairs()
{
    QString                     stem, suffix, word, prefix;
    int                         suffix_length, prefix_length;
    map_string_to_word_ptr_iter *   word_iter = m_Words->get_iterator();
    bool                        DoNotParseCompoundsFlag = true;
    while (word_iter->hasNext())   {
        word = word_iter->next().value()->GetWord();
        if (DoNotParseCompoundsFlag and word.contains("-")){
            *m_Compounds << word;
            continue;
        }
        for (int letterno = 1; letterno < word.length(); letterno++){
            if (m_SuffixesFlag){
                stem = word.left(letterno);
                if (m_Protostems.contains(stem)){
                    suffix_length = word.length() - letterno;
                    suffix = word.right(suffix_length);
                    m_Parses->append(QPair<QString,QString>(stem,suffix));
                    if (m_Words->contains(stem)){
                        m_Parses->append(QPair<QString,QString>(stem,QString("NULL")));
                    }
                }
            }else{
                stem = word.right(letterno);
                if (m_Protostems.contains(stem)){
                    prefix_length = word.length() - letterno;
                    prefix = word.left(prefix_length);
                    m_Parses->append(QPair<QString,QString>(prefix,stem));
                    if (m_Words->contains(stem)){
                        m_Parses->append(QPair<QString,QString>(QString("NULL"),stem));

                    }
                }
            } // end of prefixes.
        }
    }
        qDebug() << "Finished finding stem/affix pairs.";
}

/*!
 * This is the third of the three initial parts of finding signatures.
 * This creates signatures, which in turn creates stems and affixes.
 */
void   CLexicon::AssignSuffixesToStems()
{   const int MINIMUM_NUMBER_OF_STEMS = 2;
    CWord *                     pWord;
    QPair<QString,QString>      this_pair;
    CSignature*                 pSig;
    QString                     this_stem_t, this_suffix, this_prefix, this_affix, this_signature_string, this_word;
    stem_list       *           p_this_stem_list;
    suffix_set *                this_ptr_to_suffix_set;
    affix_set *                 this_ptr_to_affix_set;
    CStem*                      pStem;
    map_sigstring_to_suffix_set      temp_stems_to_affix_set;
    map_sigstring_to_stem_list        temp_signatures_to_stems;
    morph_set *                 pSet;

    //--> We establish a temporary map from stems to sets of affixes as we iterate through parses. <--//
    for (int parseno = 0; parseno < m_Parses->size(); parseno++){
        this_pair = m_Parses->at(parseno);
        if (m_SuffixesFlag){
            this_stem_t = this_pair.first;
            this_suffix = this_pair.second;
        } else{
            this_stem_t = this_pair.second;
            this_suffix = this_pair.first;
        }
        if (! temp_stems_to_affix_set.contains(this_stem_t)){
            if (m_SuffixesFlag){
                pSet = new suffix_set();
            } else{
                pSet = new prefix_set();
            }
            temp_stems_to_affix_set.insert(this_stem_t,pSet);
        }
        temp_stems_to_affix_set.value(this_stem_t)->insert(this_suffix);
            }
    qDebug() << "Step 1.";
    //--> We iterate through these stems and for each stem, create QStringLists of their affixes. <--//
    //--> then we create a "pre-signature" in a map that points to lists of stems. <--//
    QMapIterator<QString, morph_set*>   stem_iter(temp_stems_to_affix_set);                       // part 1
    while (stem_iter.hasNext())                                                                  // make a presignature for each stem.
    {    stem_iter.next();
         this_stem_t            = stem_iter.key();
         this_ptr_to_affix_set  = stem_iter.value();
         QStringList temp_presignature;


         affix_set_iter affix_iter (*this_ptr_to_affix_set);
         while (affix_iter.hasNext()){
                 temp_presignature.append ( affix_iter.next() );
         }

         temp_presignature.sort();
         sigstring_t this_signature_string = temp_presignature.join("=");
         if ( ! temp_signatures_to_stems.contains(this_signature_string)){
            stem_list * pStemSet = new stem_list;
            temp_signatures_to_stems[this_signature_string] = pStemSet;
         }
         temp_signatures_to_stems.value(this_signature_string)->append(this_stem_t);
    }
        qDebug() << "Step 2.";
    //-->  create signatures, stems, affixes:  <--//
    QMapIterator<sigstring_t, stem_list*> iter_sigstring_to_stems ( temp_signatures_to_stems);
    while (iter_sigstring_to_stems.hasNext())
    {   iter_sigstring_to_stems.next();
        this_signature_string    = iter_sigstring_to_stems.key();
        p_this_stem_list         = iter_sigstring_to_stems.value();
        this_stem_t =  p_this_stem_list->first();
        affix_set this_affix_set = QSet<QString>::fromList( this_signature_string.split("="));
        //qDebug() << this_signature_string << 262;
        if (p_this_stem_list->size() >= MINIMUM_NUMBER_OF_STEMS)
        {  if( m_SuffixesFlag) {
                pSig = *m_Signatures       << this_signature_string;
            } else {
                pSig = *m_PrefixSignatures << this_signature_string;
                pSig->set_suffix_flag(false);
            }
            pSig->add_memo("Pass 1");
            //qDebug() << this_signature_string << 265 ;
            QSetIterator<suffix_t> affix_iter(this_affix_set);
            while(affix_iter.hasNext()){
                  this_affix = affix_iter.next();
                  if (m_SuffixesFlag){
                      CSuffix* pSuffix = m_Suffixes->find_or_add(this_affix);
                      pSuffix->increment_count();
                      pSig->add_affix_ptr(pSuffix);
                  }else{
                      CPrefix* pPrefix = m_Prefixes->find_or_add(this_affix);
                      pPrefix->increment_count();
                      pSig->add_affix_ptr(pPrefix);

                  }

            }
            //qDebug() << "Step 2.1";
            // --> We go through this sig's stems and reconstitute its words. <--//
            stem_list_iterator stem_iter(*p_this_stem_list);
            while (stem_iter.hasNext()){
                this_stem_t = stem_iter.next();
                pStem = m_Stems->find_or_add(this_stem_t);
                pStem->add_signature (this_signature_string);
                pSig->add_stem_pointer(pStem);
                pStem->add_memo ("Pass1= ");
                QStringList affixes = this_signature_string.split("=");
                foreach (affix_t this_affix,  affixes){
                    if (this_affix == "NULL"){
                        this_word = this_stem_t;
                    } else{
                        m_SuffixesFlag ?
                            this_word = this_stem_t + this_affix :
                            this_word = this_affix + this_stem_t ;
                    }
                    //qDebug() << this_word;
                    CWord* pWord = m_Words->get_word(this_word);
                    pWord->add_stem_and_signature(pStem,pSig);
                    pWord->add_to_autobiography("Pass1= " + this_stem_t + "=" + this_signature_string);
                    //qDebug() << pWord->get_key();
                }

            }
            //qDebug() << 312;
        }else{
            //qDebug() << 315;
            this_signature_string =  iter_sigstring_to_stems.key();
            pSig =  *m_ParaSignatures << this_signature_string;
            pStem = *m_ResidualStems << this_stem_t;
            pSig->add_stem_pointer(pStem);
            //qDebug() << 320;
            foreach (this_affix, this_affix_set){
                if (this_affix == "NULL"){
                    this_word = this_stem_t;
                } else{
                    m_SuffixesFlag ?
                       this_word = this_stem_t + this_affix :
                       this_word = this_affix + this_stem_t;
                }
                //qDebug() << 329;
                pWord = m_Words->find_or_fail(this_word);
               // qDebug() << 331;
                if (pWord){  // *** why does this sometimes fail?
                    //qDebug()  <<333;
                    pWord->add_to_autobiography("*" + this_stem_t + "=" + this_signature_string);
                    //qDebug() << 334;
                }
            }
         }
    }
    m_Suffixes->sort_by_count();
    qDebug() << "step 4 Finished finding signatures.";
}

bool contains(QList<QString> * list2, QList<QString> * list1){
    for (int i=0; i < list1->length();i++){
        bool success = false;
        for (int j = 0; j < list2->length();j++){
            if (list1->at(i) == list2->at(j)){
                success = true;
            }
        }
        if (success == false){
            return false;
        }
    }
    return true;
}


/*!
 * We look inside the ParaSignatures, and extract only the approved suffixes inside them.
 */
void   CLexicon::FindGoodSignaturesInsideParaSignatures()
{   stem_t                      this_stem;
    word_t                      this_word;
    suffix_t                    this_suffix;
    sig_string                  this_signature_string;
    CStem*                      pStem, *qStem;
    CSuffix*                    pSuffix;
    CWord*                      pWord;
    CSignature*                 pSig, *p_proven_sig;
    CSuffix*                    pSuffix1;
    int                         suffix_no;
    int                         count_of_new_stems = 0;
    int                         count_of_new_words = 0;
    int                         signature_count (0);
    //suffix_list               true_suffix_list;
    suffix_list                 suffixes_of_residual_sig;
    CSuffix_ptr_list  *         list_of_CSuffixes_of_proven_sig;
    CSuffix_ptr_list            this_residual_sig_suffix_pointer_list;
    bool                        success_flag;

    suffix_t                    Null_string ("NULL");
    CSuffix*                    pNullSuffix = *m_Suffixes << Null_string;

                                m_ProgressBar->reset();
                                m_ProgressBar->setMinimum(0);
                                m_ProgressBar->setMaximum(m_ParaSignatures->get_count());
                                m_Signatures->sort_signatures_by_affix_count();
    //---->   We iterate through the list of Residual Signatures <-------//

    map_sigstring_to_sig_ptr_iter   sig_iter(*  m_ParaSignatures->get_map());

    //--> Outer loop, over all Residual Signatures. <--//
    while (sig_iter.hasNext()){
        sig_iter.next();
        signature_count++;
        m_ProgressBar->setValue(signature_count);
        qApp->processEvents();
        CSignature*         pResidualSig              = sig_iter.value();
                            suffixes_of_residual_sig  = pResidualSig->get_key().split("=");
                            this_stem = pResidualSig->get_stems()->first()->get_key(); // there is only 1 stem in these signatures, by construction.
                            if (m_Words->contains(this_stem)){
                              suffixes_of_residual_sig.append ("NULL");
                            }
                            //--> Now we look for largest true signature inside this list of suffixes. <--//
                            //--> Inner loop, over all good signatures. <--//

                            for (int sig_no=0; sig_no < get_signatures()->get_count(); sig_no++){
                                p_proven_sig = m_Signatures->get_at_sorted(sig_no);
                                QString p_proven_sigstring  = p_proven_sig->get_key();
                                QList<QString> proven_sig_list = p_proven_sigstring.split("=");
                                if ( contains(&suffixes_of_residual_sig, &proven_sig_list) ){

                                    // We have found the longest signature contained in this_residual_suffix_set

                                    pStem = m_Stems->find_or_add(this_stem);
                                    p_proven_sig->add_stem_pointer(pStem);
                                    // pStem->add_memo("singleton=");
                                    //--> add to autobiographies <--//

                                    for (int affixno = 0; affixno < proven_sig_list.length(); affixno++){
                                        this_suffix = proven_sig_list[affixno];
                                        if (this_suffix == "NULL"){
                                            this_word= this_stem;
                                        }else{
                                            this_word = this_stem + this_suffix;
                                        }
                                        pWord = m_Words->find_or_fail(this_word);
                                        if (this_word=="Poisons"){qDebug()<< "Poisons"<< 430;}
                                        if (pWord){
                                            if (this_word == "Poisons"){qDebug() << " found word entry."<< 432;}
                                            pWord->add_stem_and_signature(pStem, p_proven_sig);
                                            pWord->add_to_autobiography("from within parasigs "  + this_stem  + "=" +  p_proven_sigstring);
                                        }
                                    }
                                    break;
                                } else{

                                }
                            } // loop over proven signatures;

    }
    m_Signatures->sort_each_signatures_stems_alphabetically();
}


/*!
 *  This function looks at all the non-signatures that were rejected
 *  because they were associated with only one stem. For each one, it
 *  looks for the broadest signature that occurs inside it, and assigns
 *  its stem to that signature.
 */

struct{
    bool operator ()(CSignature* pSig_a, CSignature* pSig_b) const {
    // return pSig_a->number_of_true_suffixes() > pSig_b->number_of_true_suffixes();
    }
}custom_compare_residual_sig;



/*!
 * Replace parse pairs from current signature structure. This allows us to
 * delete the old signature structure, and replace them with the set of
 * parse-pairs that exactly describe the current signature structure.
 */

void CLexicon::replace_parse_pairs_from_current_signature_structure(bool FindSuffixesFlag) {
    m_Parses->clear();
    m_Parse_map.clear();
    QString                         sig_string;
    CSignature*                     pSig;
    CStem*                          pStem;
    QList<CStem*> *                 stem_list;
    map_sigstring_to_sig_ptr_iter  * sig_iter =  get_signatures()->get_map_iterator();

    while (sig_iter->hasNext()){
        pSig = sig_iter->next().value();
        stem_list =  pSig->get_stems();
    }
}





/*!
 * We can build a graph whose nodes are the signatures, where an edge connects
 * any pair of signatures if there exists a word that they both analyze.
 * A sig_tree_edge has two flavors: this function uses the flavor that
 * contains the two signatures, the word, and the string-difference between
 * the stems of the word at the two signatures.
 */
void CLexicon::compute_sig_tree_edges()
{
    CWord*                          pWord;
    map_string_to_word *            WordMap = m_Words->GetMap();
    map_string_to_word_ptr_iter         word_iter(*WordMap);
    simple_sig_tree_edge *                 p_SigTreeEdge;
    int                             number_of_signatures;
    CSignatureCollection*           pSignatures;
    morph_t                         difference;
    while (word_iter.hasNext())   {
        pWord = word_iter.next().value();
        number_of_signatures = pWord->GetSignatures()->size();
        if ( number_of_signatures > 2){
            for (int signo1=0; signo1 < number_of_signatures; signo1++){
                stem_sig_pair* pair1 =  pWord->GetSignatures()->value(signo1);
                CStem * stem1        = pair1->first;
                int stem1length      = stem1->get_key().length();
                CSignature* sig1     = pair1->second;
                for (int signo2=signo1 + 1; signo2 < number_of_signatures; signo2++){
                    stem_sig_pair * pair2 = pWord->GetSignatures()->value(signo2);
                    CStem *  stem2   = pair2->first;
                    CSignature* sig2 = pair2->second;
                    if (sig1 == sig2){continue;}
                    int stem2length = stem2->get_key().length();
                    if (stem1length > stem2length){
                        int length_of_difference = stem1length - stem2length;
                        m_SuffixesFlag?
                            difference = stem1->get_key().mid(stem2->get_key().length()):
                            difference = stem1->get_key().left(length_of_difference);
                        p_SigTreeEdge = new simple_sig_tree_edge (sig1,sig2,difference, pWord->get_key(), stem1->get_key(), stem2->get_key());
                    } else{
                        int length_of_difference = stem2length - stem1length;
                        m_SuffixesFlag?
                            difference = stem2->get_key().mid(stem1->get_key().length()):
                            difference = stem2->get_key().left(length_of_difference);
                        p_SigTreeEdge =  new simple_sig_tree_edge (sig2,sig1,difference, pWord->get_key(), stem1->get_key(), stem2->get_key());
                    }
                    //qDebug() << sig1->get_key()<<sig2->get_key() << difference << pWord->get_key() << stem1->get_key() << 530;
                    m_SigTreeEdgeList.append(p_SigTreeEdge);
                }
            }
        }
    }
}

/*!
 * This function takes the SigTreeEdge list, and makes a smaller list composed of
 * SigTreeEdges which share the same signature pair and string-difference. This
 * flavor of sig_tree_edge contains a list of all the words that participate in
 * this particular sig_tree_edge.
 */
void CLexicon::compute_sig_tree_edge_map() {
morph_t         edge_label;
word_t          this_word;
simple_sig_tree_edge * p_sig_tree_edge;
sig_tree_edge        * p_sig_tree_edge_2,
                     * p_sig_tree_edge_3;
QMap<QString, sig_tree_edge*> * p_EdgeMap = & m_SigTreeEdgeMap;

QListIterator<simple_sig_tree_edge*> this_simple_sig_tree_edge_iter (m_SigTreeEdgeList);
while (this_simple_sig_tree_edge_iter.hasNext())
{
    p_sig_tree_edge = this_simple_sig_tree_edge_iter.next();
    edge_label = p_sig_tree_edge->label();
    this_word  = p_sig_tree_edge->word;
    //qDebug() << this_word << 558;
    // --> We iterate through the simple Edges contained in the TreeEdge List            <-- //
    // --> We build a map of larger TreeEdges, each containing multiple stems and words. <-- //
    if (p_EdgeMap->contains(edge_label)){
        //qDebug() << "we found an edge that will be reused "<< this_word;
        p_sig_tree_edge_3 = p_EdgeMap->value(edge_label);
        word_stem_struct * this_word_stem_struct = new word_stem_struct;
        this_word_stem_struct->word = this_word;
        this_word_stem_struct->stem_1 = p_sig_tree_edge->stem_1;
        this_word_stem_struct->stem_2 = p_sig_tree_edge->stem_2;

        if ( ! p_sig_tree_edge_3->shared_word_stems.contains(this_word_stem_struct)){
            p_sig_tree_edge_3->shared_word_stems.append(this_word_stem_struct);
        }

    } else {  // --> start a new sig_tree_edge with multiple stems <-- //
        // change this to a constructor that takes a simple-tree-edge as its argument
        //qDebug() << this_word << 575 << "new big edge";
        sig_tree_edge * p_sig_tree_edge_2 = new sig_tree_edge(
            p_sig_tree_edge->sig_1,
            p_sig_tree_edge->sig_2,
            p_sig_tree_edge->morph,
            p_sig_tree_edge->word,
            p_sig_tree_edge->stem_1,
            p_sig_tree_edge->stem_2
           );
        m_SigTreeEdgeMap.insert(p_sig_tree_edge_2->label(),p_sig_tree_edge_2);
    }
}
}


void CLexicon::dump_suffixes(QList<QString> * pList)
{
    return m_Suffixes->get_suffixes(pList);
}

void CLexicon::collect_parasuffixes()
{   sigstring_t     sigstring;
    suffix_t        suffix;
    CSignature*     pSig;
    CSuffix *       pSuffix;
    QStringList     suffixes;
    map_sigstring_to_sig_ptr_iter sig_iter (* m_ParaSignatures->get_map());
    while (sig_iter.hasNext())
    { pSig = sig_iter.next().value();
        sigstring = pSig->get_key();
        suffixes = sigstring.split("=");
        foreach (suffix, suffixes){
            pSuffix = *m_ParaSuffixes <<  suffix;
            pSuffix->increment_count();
        }
    }
    m_ParaSuffixes->sort_by_count();
}









 /*!
 * This is lke AssignSuffixesToStems, but with Stems known ahead of time.
 * This creates signatures and stems.
 */
void CLexicon::ReSignaturizeWithKnownAffixes()

{   const int MINIMUM_NUMBER_OF_STEMS = 2;
    CWord *                     pWord;
    QPair<QString,QString>      this_pair;
    CSignature*                 pSig;
    QString                     this_stem_t, this_suffix, this_prefix, this_affix, this_signature_string, this_word;
    stem_list       *           p_this_stem_list;
    suffix_set *                this_ptr_to_suffix_set;
    affix_set *                 this_ptr_to_affix_set;
    CStem*                      pStem;
    map_sigstring_to_suffix_set      temp_stems_to_affix_set;
    map_sigstring_to_stem_list        temp_signatures_to_stems;
    morph_set *                 pSet;

    map_string_to_word_ptr_iter word_iter (*m_Words->get_map());
    while(word_iter.hasNext()){
        pWord = word_iter.next().value();
        pWord->clear_signatures();
    }
    //--> We establish a temporary map from stems to sets of affixes as we iterate through parses. <--//
    for (int parseno = 0; parseno < m_Parses->size(); parseno++){
        this_pair = m_Parses->at(parseno);
        if (m_SuffixesFlag){
            this_stem_t = this_pair.first;
            this_suffix = this_pair.second;
            if (! m_Suffixes->contains(this_suffix)){
                continue;
            }
        } else{
            this_stem_t = this_pair.second;
            this_suffix = this_pair.first;
            if (! m_Prefixes->contains(this_suffix)){
                continue;
            }
        }

        if (! temp_stems_to_affix_set.contains(this_stem_t)){
            if (m_SuffixesFlag){
                pSet = new suffix_set();
            } else{
                pSet = new prefix_set();
            }
            temp_stems_to_affix_set.insert(this_stem_t,pSet);
        }
        temp_stems_to_affix_set.value(this_stem_t)->insert(this_suffix);
            }

    //--> We iterate through these stems and for each stem, create QStringLists of their affixes. <--//
    //--> then we create a "pre-signature" in a map that points to lists of stems. <--//
    QMapIterator<QString, morph_set*>   stem_iter(temp_stems_to_affix_set);                       // part 1
    while (stem_iter.hasNext())                                                                  // make a presignature for each stem.
    {    stem_iter.next();
         this_stem_t            = stem_iter.key();
         this_ptr_to_affix_set  = stem_iter.value();
         if (this_ptr_to_affix_set->size() < 2){continue;}
         QStringList temp_presignature;


         affix_set_iter affix_iter (*this_ptr_to_affix_set);
         while (affix_iter.hasNext()){
                 temp_presignature.append ( affix_iter.next() );
         }

         temp_presignature.sort();
         sigstring_t this_signature_string = temp_presignature.join("=");
         if ( ! temp_signatures_to_stems.contains(this_signature_string)){
            stem_list * pStemSet = new stem_list;
            temp_signatures_to_stems[this_signature_string] = pStemSet;
         }
         temp_signatures_to_stems.value(this_signature_string)->append(this_stem_t);
    }

    //-->  create signatures, stems, affixes:  <--//
    m_Stems->clear();
    if (m_SuffixesFlag){
        m_Signatures->clear();
    }else {
        m_PrefixSignatures->clear();
    }
    QMapIterator<sigstring_t, stem_list*> iter_sigstring_to_stems ( temp_signatures_to_stems);
    while (iter_sigstring_to_stems.hasNext())
    {   iter_sigstring_to_stems.next();
        this_signature_string    = iter_sigstring_to_stems.key();
        p_this_stem_list         = iter_sigstring_to_stems.value();
        this_stem_t =  p_this_stem_list->first();
        affix_set this_affix_set = QSet<QString>::fromList( this_signature_string.split("="));
        if (p_this_stem_list->size() >= MINIMUM_NUMBER_OF_STEMS)
        {   m_SuffixesFlag ?
                pSig = *m_Signatures       << this_signature_string :
                pSig = *m_PrefixSignatures << this_signature_string;
            pSig->add_memo("Pass 2");
            QSetIterator<suffix_t> affix_iter(this_affix_set);
            while(affix_iter.hasNext()){
                  this_affix = affix_iter.next();
                  if (m_SuffixesFlag){
                      CSuffix* pSuffix = m_Suffixes->find_or_add(this_affix);
                      pSuffix->increment_count();
                      pSig->add_affix_ptr(pSuffix);
                  }else{
                      CPrefix* pPrefix = m_Prefixes->find_or_add(this_affix);
                      pPrefix->increment_count();
                      pSig->add_affix_ptr(pPrefix);
                  }
            }

            // --> We go through this sig's stems and reconstitute its words. <--//
            stem_list_iterator stem_iter(*p_this_stem_list);
            while (stem_iter.hasNext()){
                this_stem_t = stem_iter.next();
                pStem = m_Stems->find_or_add(this_stem_t);
                pStem->add_signature (this_signature_string);
                pSig->add_stem_pointer(pStem);
                pStem->add_memo ("Pass1= ");
                QStringList affixes = this_signature_string.split("=");
                foreach (affix_t this_affix,  affixes){
                    if (this_affix == "NULL"){
                        this_word = this_stem_t;
                    } else{
                        m_SuffixesFlag ?
                            this_word = this_stem_t + this_affix :
                            this_word = this_affix + this_stem_t ;
                    }
                    CWord* pWord = m_Words->get_word(this_word);
                    pWord->add_stem_and_signature(pStem,pSig);
                    pWord->add_to_autobiography("Pass2= " + this_stem_t + "=" + this_signature_string);
                }
            }

        }else{
            this_signature_string =  iter_sigstring_to_stems.key();
            pSig =  *m_ParaSignatures << this_signature_string;
            pStem = *m_ResidualStems << this_stem_t;
            pSig->add_stem_pointer(pStem);
            foreach (this_affix, this_affix_set){
                if (this_affix == "NULL"){
                    this_word = this_stem_t;
                } else{
                    m_SuffixesFlag ?
                       this_word = this_stem_t + this_affix :
                       this_word = this_affix + this_stem_t;
                }
                pWord = m_Words->find_or_fail(this_word);
                pWord->add_to_autobiography("*" + this_stem_t + "=" + this_signature_string);
            }
        }
    }
    m_Signatures->sort_each_signatures_stems_alphabetically();
}
