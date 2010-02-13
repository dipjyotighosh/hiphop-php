/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __GENERATED_cls_recursivedirectoryiterator_h__
#define __GENERATED_cls_recursivedirectoryiterator_h__

#include <cls/directoryiterator.h>
#include <cls/recursiveiterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/iterator.php line 209 */
class c_recursivedirectoryiterator : virtual public c_directoryiterator, virtual public c_recursiveiterator {
  BEGIN_CLASS_MAP(recursivedirectoryiterator)
    PARENT_CLASS(splfileinfo)
    PARENT_CLASS(traversable)
    PARENT_CLASS(iterator)
    PARENT_CLASS(traversable)
    PARENT_CLASS(seekableiterator)
    PARENT_CLASS(directoryiterator)
    PARENT_CLASS(traversable)
    PARENT_CLASS(iterator)
    PARENT_CLASS(recursiveiterator)
  END_CLASS_MAP(recursivedirectoryiterator)
  DECLARE_CLASS(recursivedirectoryiterator, RecursiveDirectoryIterator, directoryiterator)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: void t___construct(Variant v_path, Variant v_flags = 16LL /* recursivedirectoryiterator::CURRENT_AS_FILEINFO */);
  public: ObjectData *create(Variant v_path, Variant v_flags = 16LL /* recursivedirectoryiterator::CURRENT_AS_FILEINFO */);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: Variant t_current();
  public: Variant t_key();
  public: Variant t_next();
  public: Variant t_rewind();
  public: Variant t_seek(CVarRef v_position);
  public: String t___tostring();
  public: bool t_valid();
  public: bool t_haschildren();
  public: Object t_getchildren();
  public: String t_getsubpath();
  public: String t_getsubpathname();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_recursivedirectoryiterator_h__