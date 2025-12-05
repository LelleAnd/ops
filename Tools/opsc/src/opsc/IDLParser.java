/** -*-java-*- Copyright (C) 2014 Saab Dynamics AB
 ***************************************************************************
 *  @file   IDLParser.java
 *  @author Mattias Helsing <mattias.helsing@saabgroup.com>
 *          Updated/Modified by Lennart Andersson
 *
 * This file is based on:
 *   Tools/NBOPSIDLSupport/src/ops/netbeansmodules/idlsupport/ProjectIDLParser.java
 ***************************************************************************
 */

package opsc;

import java.io.IOException;
import java.util.Vector;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.ParseException;

/**
 *
 * @author helm
 */
public class IDLParser
{
    Vector<IDLClass> _idlClasses = new Vector<IDLClass>();
    Vector<IDLClass> _idlRefClasses = new Vector<IDLClass>();

    // FileParser from IDLParser jar
    parsing.javaccparser.FileParser _parser = new parsing.javaccparser.FileParser();

    private int _nrErrors = 0;
    private int _nrWarnings = 0;

    public IDLParser()
    {
    }

    public void reset()
    {
        _idlClasses.clear();
        _idlRefClasses.clear();
        _nrErrors = 0;
        _nrWarnings = 0;
    }

    public IDLClass parse(String name, String fileText, boolean refClass)
    {
        try {
            // Parse file
            IDLClass newClass = _parser.parse(fileText);

            // Set some class info from directives
            newClass.setTopLevel(CompilerSupport.is_TopLevel(newClass));
            newClass.setNoFactory(CompilerSupport.is_NoFactory(newClass));
            newClass.setOnlyDefinition(CompilerSupport.is_OnlyDefinition(newClass));

            // Perform some post parsing checks
            // First check that file name and class name is the same
            if (newClass.getClassName().equals(name)) {
                // Next check that onlydefinition classes don't have fields
                if (newClass.isOnlyDefinition()) {
                    for (IDLField field : newClass.getFields()) {
                        if (field.isStatic()) continue;
                        System.out.println("Error: File " + name + " set as 'onlydefinition' but contains fields");
                        _nrErrors++;
                        return null;
                    }
                }

                // Passed all checks, add to list of parsed IDL classes
                if (refClass) {
                    _idlRefClasses.add(newClass);
                } else {
                    _idlClasses.add(newClass);
                }
                return newClass;

            } else {
                System.out.println("In " + name + ", Error: File " + name + " does not contain a class called " + name + ".");
                _nrErrors++;
            }
        } catch (ParseException ex) {
            System.out.println("In " + name + ", Error: " + ex.getMessage());
            _nrErrors++;
        }
        return null;
    }

    protected IDLClass findClass(Vector<IDLClass> idlClasses, String pkgName, String clsName)
    {
      if (clsName == null) { return null; }
      // If name contains a '.' it contains a package name
      String packageName = pkgName;   // Default
      String className = clsName;
      int idx = className.lastIndexOf('.');
      if (idx > 0) {
          packageName = className.substring(0,idx);
          className = className.substring(idx+1);
      }
      for (IDLClass idlClass : idlClasses) {
          if (idlClass.getClassName().equals(className) && idlClass.getPackageName().equals(packageName)) {
              return idlClass;
          }
      }
      return null;
    }

    // Connect classes to base class
    public void connectClasses()
    {
      // For ref classes, check other classes if not found else where
      for (IDLClass idlClass : _idlRefClasses) {
        idlClass.setBaseClassRef(findClass(_idlRefClasses, idlClass.getPackageName(), idlClass.getBaseClassName()));
        if (idlClass.getBaseClassRef() == null) {
          idlClass.setBaseClassRef(findClass(_idlClasses, idlClass.getPackageName(), idlClass.getBaseClassName()));
        }
      }
      // For other classes, check ref classes if not found else where
      for (IDLClass idlClass : _idlClasses) {
        idlClass.setBaseClassRef(findClass(_idlClasses, idlClass.getPackageName(), idlClass.getBaseClassName()));
        if (idlClass.getBaseClassRef() == null) {
          idlClass.setBaseClassRef(findClass(_idlRefClasses, idlClass.getPackageName(), idlClass.getBaseClassName()));
        }
      }
    }

    public Vector<IDLClass> getIdlClasses()
    {
        return _idlClasses;
    }

    public int getNrErrors()
    {
        return _nrErrors;
    }

    public int getNrWarnings()
    {
        return _nrWarnings;
    }
}
