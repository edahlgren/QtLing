#ifndef EVALUATION_H
#define EVALUATION_H

#include "Word.h"
class MainWindow;
class Lexicon;
class CWordCollection;
class EvalParses;


/*!
 * \brief A class that stores the output results of evaluation using
 * a Gold Standard.
 *
 * Source code for functions in evaluation.cpp.
 */
class EvaluationResults
{
    double  m_total_recall;         //!< Recall.
    double  m_total_precision;      //!< Precision.
    int     m_gs_word_count;        //!< Number of words in Gold Standard.
    int     m_retrieved_word_count; //!< Number of words in evaluated set of parses.
    int     m_overlap_word_count;   //!< Number of words existing both in Gold Standard and evaluated set of data.
    int     m_true_positive_count;  //!< Total number of parses that appear both in Gold Standard and
                                    //!< evaluated set.
    int     m_gs_parse_count;       //!< Total number of parses of only the overlapping words,
                                    //!< given by Gold Standard.
    int     m_retrieved_parse_count;//!< Total number of parses of only the overlapping words,
                                    //!< found in the evaluated set of data.
public:
    EvaluationResults();
    // use default constructor, copy constructor, assignment, and destructor

    void        set_gs_word_count(int x)            {m_gs_word_count = x;}
    void        set_retrieved_word_count(int x)     {m_retrieved_word_count = x;}
    void        set_overlap_word_count(int x)       {m_overlap_word_count = x;}
    void        set_true_positive_count(int x)      {m_true_positive_count = x;}
    void        set_gs_parse_count(int x)           {m_gs_parse_count = x;}
    void        set_retrieved_parse_count(int x)    {m_retrieved_parse_count = x;}
    int         get_gs_word_count()                 {return m_gs_word_count;}
    int         get_retrieved_word_count()          {return m_retrieved_word_count;}
    int         get_overlap_word_count()            {return m_overlap_word_count;}
    int         get_true_positive_count()           {return m_true_positive_count;}
    int         get_gs_parse_count()                {return m_gs_parse_count;}
    int         get_retrieved_parse_count()         {return m_retrieved_parse_count;}
    double      get_total_recall()                  {return m_total_recall;}
    double      get_total_precision()               {return m_total_precision;}

    void        clear();
    void        calculate_precision_recall();
};

/*!
 * \brief A handler class for `QMap<QString, QMap<QString, Parse_triple*>*>`.
 *
 * Source code for functions in evaluation_parsemap.cpp.
 */
class ParseMapHandler
{
public:
    typedef QMap<QString, Parse_triple*> Parse_triple_map;
    typedef QMap<QString, Parse_triple_map*> ParseMap;

protected:
    ParseMap*       m_map;

    ParseMap*       clone_map() const;
    void            destroy_map();
public:
    ParseMapHandler();
    ParseMapHandler(const ParseMapHandler& parsemap);
    ParseMapHandler& operator=(const ParseMapHandler& parsemap);
    ~ParseMapHandler();

    ParseMap&       operator*()             { return *m_map; }
    ParseMap*       operator->()            { return m_map; }

    ParseMap*       get_map()               { return m_map; }
    // void            get_parse_triple_map(const QString& word);

    void            add_parse_triple(const QString& word, const QString& stem, const QString& affix);
};

/*!
 * \brief A class that reads in and stores parses of words from a Gold Standard
 * XML file produced by Alchemist. Contains functions to evaluate other parsing
 * algorithms using Gold Standard parses.
 *
 * Source code for functions in evaluation_goldstandard.cpp.
 */
class GoldStandard
{
friend class EvalParses;
public:
    typedef QMap<QString, Parse_triple*> Parse_triple_map;
    typedef QMap<QString, Parse_triple_map*> ParseMap;

protected:
    EvaluationResults   m_results;              //!< Data structure to store results.
    ParseMapHandler     m_true_positive_parses; //!< Collection of "true positive" parses.
    ParseMapHandler     m_retrieved_parses;     //!< Collection of parses found by Linguistica.

    QString     m_XML_directory;
    QString     m_XML_file_name; //!< File name of Gold Standard XML document.

    ParseMapHandler m_gs_parses; //!< Collection of parses given by Gold Standard.

public:
    GoldStandard();
    GoldStandard(const GoldStandard& gs);
    explicit GoldStandard(QString& XML_file_name);
    GoldStandard& operator=(const GoldStandard& gs);
    ~GoldStandard();

    bool                read_XML();
    bool                evaluate(CWordCollection* p_word_collection);
    bool                evaluate(CWordCollection* p_word_collection,
                                 EvaluationResults& results,
                                 ParseMapHandler* p_true_positive_parses = NULL,
                                 ParseMapHandler* p_retrieved_parses = NULL);
    bool                evaluate(EvalParses* p_eval_parses);
    bool                evaluate(ParseMapHandler& eval_parses,
                                EvaluationResults& results,
                                ParseMapHandler* p_true_positive_parses = NULL,
                                ParseMapHandler* p_retrieved_parses = NULL);

    double              get_total_recall()          { return m_results.get_total_recall(); }
    double              get_total_precision()       { return m_results.get_total_precision(); }

    EvaluationResults&  get_results()               { return m_results; }

    ParseMapHandler     get_true_positive_parses()  { return m_true_positive_parses; }
    ParseMapHandler     get_retrieved_parses()      { return m_retrieved_parses; }
    ParseMapHandler     get_gs_parses()             { return m_gs_parses; }

};


/*!
 * \brief A class that reads in and stores parses of words from a txt file produced
 * by Morfessor. Stores results after these parses are evaluated against
 * a Gold Standard.
 *
 * Source code for functions in evaluation_evalparses.cpp.
 */
class EvalParses
{
    friend class GoldStandard;
public:
    typedef QMap<QString, Parse_triple*> Parse_triple_map;
    typedef QMap<QString, Parse_triple_map*> ParseMap;
protected:
    ParseMapHandler         m_parses;
    EvaluationResults       m_results;

    QString                 m_file_name;
    bool                    m_evaluated;


public:
    EvalParses();
    explicit EvalParses(QString& file_name);
    EvalParses(const EvalParses& eval);
    EvalParses& operator=(const EvalParses& eval);
    ~EvalParses();


    ParseMapHandler         get_parses()                {return m_parses;}

    double                  get_total_precision()       {return m_results.get_total_precision();}
    double                  get_total_recall()          {return m_results.get_total_recall();}

    EvaluationResults&      get_results()               { return m_results; }

    bool                    evaluate(GoldStandard* p_gs);

    bool                    is_evaluated()              {return m_evaluated;}

    bool                    read_morfessor_txt_file();



};

#endif // EVALUATION_H
