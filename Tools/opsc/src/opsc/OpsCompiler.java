/** -*-java-*- Copyright (C) 2014 Saab Dynamics AB
 ***************************************************************************
 *  @file   opsc/OpsCompiler.java
 *  @author Mattias Helsing <mattias.helsing@saabgroup.com>
 *          Updated/Modified by Lennart Andersson
 *
 * An attempt to create a command line application for generating
 * Cpp files from OPS IDL files.
 *
 ***************************************************************************
 */

package opsc;

import java.io.FileOutputStream;
import java.io.BufferedReader;
import java.util.ArrayList;
import java.util.List;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileFilter;
import java.io.FileWriter;
import java.io.FileReader;
import java.util.Vector;
import java.util.HashSet;
import java.util.Iterator;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.net.URL;

import parsing.IDLClass;
import parsing.IDLField;
import parsing.IDLEnumType;
import opsc.CompilerSupport;

/**
 *
 * @author helm
 */
public class OpsCompiler
{
    List<String> _listInputFiles = new ArrayList<String>();
    List<String> _listRefFiles = new ArrayList<String>();
    String _strProjectName = "";
    String _strProjectDir = "";
    boolean _bOnlyParse = false;
    boolean _bOnlyGenFactories = false;
    boolean _bGenMemoryPool = false;
    boolean _bJsonVersion = true;
    boolean _bPythonInit = false;
    boolean _bPythonPackage = false;
    boolean _bKeepCharForByte = false;  // C++: char or uint8_t for idl byte
    String _strOps4GprPath = "";

    /** An instance of ProjectProperties is used to hold defaults
     *  as well is modifications to defaults either read from the
     *  command-line or project files or what have you */
    private ProjectProperties _props = new ProjectProperties();

    /** The IDL parser creates IDLClass instances from idl files */
    private opsc.IDLParser _parser;

    /** A verbosity flag. */
    int _verbose = 0;
    boolean _dumpFlag = false;

    /** The java generator instance */
    protected opsc.JavaCompiler _javaCompiler;
    protected opsc.CSharpCompiler _CSharpCompiler;
    protected opsc.DelphiCompiler _DelphiCompiler;
    protected opsc.AdaCompiler _AdaCompiler;
    protected opsc.JSONCompiler _JSONCompiler;

    HashSet<String> _packageNamesInCompilation = new HashSet<String>();

    public OpsCompiler() {
        // create a parser to generate IDLClasses for us
        _parser = new opsc.IDLParser();
    }

    public static void usage()
    {
        System.out.println("");
        System.out.println("opsc -P <IDL_proj_dir> [options] [-ref idlfiles...]");
        System.out.println("  or");
        System.out.println("opsc [options] idlfiles... [-ref idlfiles...]");
        System.out.println("");
        System.out.println("  idlfiles...       one or more names; name can be a file with extension '.idl' or");
        System.out.println("                    a directory (ended with a path separator) to be recursively searched");
        System.out.println("                    for idlfiles");
        System.out.println("");
        System.out.println("OPTIONS");
        System.out.println("  -? | -h | --help  show this help");
        System.out.println("  -b <feature>      build given feature");
        System.out.println("  -B <feature>      don't build given feature");
        System.out.println("  -d[d]             verbose output");
        System.out.println("  -dump             print all parsed objects");
        System.out.println("  -fac              only generate factories for given features");
        System.out.println("  -g <feature>      generate given feature");
        System.out.println("  -G <feature>      don't generate given feature");
        System.out.println("  -idls <file>      get idl-filenames from given file");
        System.out.println("                    (one filename per line in file, paths relative to <file>)");
        System.out.println("  -o <dir>          set output directory");
        System.out.println("  -p <projname>     set project name");
        System.out.println("  -P <IDL_proj_dir> use as project directory with pre-defined subdirectories");
        System.out.println("  -parse            only parse, don't generate");
        System.out.println("  -pp <file>        name an ops IDL project.properties file");
        System.out.println("  -printProps       print system props");
        System.out.println("  -ref              remaining idlfiles specified are only used as reference (no code gen)");
        System.out.println("  -retain <feature> retain old behaviour for given feature");
        System.out.println("  -s <feature>      special, generate with given feature");
        System.out.println("  -S <feature>      special, don't generate with given feature");
        System.out.println("  -t <dir>          set template directory (overrides built-in templates)");
        System.out.println("");
        System.out.println("  -gpr <path>       explicit path to ops4.gpr when generating Ada");
        System.out.println("                    (default is to use GPR_PROJECT_PATH to find ops4.gpr)");
        System.out.println("  -dll <file>       used when building C# to give any dll dependencies (eg. OpsLibrary.dll)");
        System.out.println("  -jar <file>       used when building Java to give any jar dependencies");
        System.out.println("  -pypac <file>     used when generating Python packages to give any Python package dependencies (*.opsc files)");
        System.out.println("");
        System.out.println("FEATURE");
        System.out.println("  for generate: ALL, ada, cpp(*), csharp(*), delphi, java(*), json, python(*), debug");
        System.out.println("  for build:    ALL, csharp, java");
        System.out.println("  for special:  mempool, jsonver(*), pyinit, pypack");
        System.out.println("                (*) == Default enabled");
        System.out.println("  for retain:   char  -> In generated C++, use 'char' for idl byte instead of 'uint8_t'");
        System.out.println("");
    }

    /**
     * A class that implements the Java FileFilter interface.
     */
    public class IdlFileFilter implements FileFilter
    {
      public boolean accept(File file)
      {
        if (file.isDirectory()) return true;
        return file.getName().toLowerCase().endsWith("idl");
      }
    }

    // Add recursively all idl-files starting with directory 'src'
    protected void addFilesFromDirectory(File dir, Vector<String> extraArgs)
    {
      File[] files = dir.listFiles(new IdlFileFilter());
      for (File f : files) {
        if (f.isDirectory()) {
          if (f.getName().equals(".")) continue;
          if (f.getName().equals("..")) continue;
          addFilesFromDirectory(f, extraArgs);
        } else {
          if (_verbose > 0) System.out.println("Debug: Adding arg: " + f.getAbsolutePath());
          extraArgs.add(f.getAbsolutePath());
        }
      }
    }

    // Add all idl-files listed in given file (one file per line in the given file)
    protected void addFilesFromFile(File idlfile, Vector<String> extraArgs)
    {
      try {
        if (idlfile.isFile()) {
          //System.out.println("found... " + idlfile.getCanonicalPath());
          String basedir = idlfile.getParent();
          BufferedReader reader = new BufferedReader(new FileReader(idlfile.getCanonicalPath()));
          String line = reader.readLine();
          while (line != null) {
            File f = new File(basedir + File.separator + line);
            if (_verbose > 0) System.out.println("Debug: Adding arg: " + f.getAbsolutePath());
            extraArgs.add(f.getAbsolutePath());
            line = reader.readLine();
          }
        }
      } catch (java.io.IOException ioe) {
      }
    }

    protected void updateGenerateProp(String feature, boolean value)
    {
      if(feature.equals("ALL") || feature.equals("ada")) _props.generateAda = value;
      if(feature.equals("ALL") || feature.equals("cpp")) _props.generateCpp = value;
      if(feature.equals("ALL") || feature.equals("csharp")) _props.generateCS = value;
      if(feature.equals("ALL") || feature.equals("delphi")) _props.generateDelphi = value;
      if(feature.equals("ALL") || feature.equals("java")) _props.generateJava = value;
      if(feature.equals("ALL") || feature.equals("json")) _props.generateJSON = value;
      if(feature.equals("ALL") || feature.equals("python")) _props.generatePython = value;
      if(feature.equals("ALL") || feature.equals("debug")) _props.buildDebugProject = value;
    }

    protected void updateBuildProp(String feature, boolean value)
    {
      if(feature.equals("ALL") || feature.equals("csharp")) _props.buildCS = value;
      if(feature.equals("ALL") || feature.equals("java")) _props.buildJava = value;
    }

    protected boolean parseCommandLineArgs(String args[])
    {
        //System.out.println("Info: Parse arguments...");

        Vector<String> extraArgs = new Vector<String>();
        Vector<String> tmpFiles = new Vector<String>();
        boolean projDirGiven = false;

        // first step in handling arguments. Some of these will expand the command line.
        for (int i = 0 ; i < args.length ; i++) {
          if (args[i].equals("-d")) {
            _verbose = 1;

          } else if (args[i].equals("-dd")) {
            _verbose = 2;

          // find out if we have idls given using files
          } else if (args[i].equals("-idls") && (i < args.length)) {
            i++;
            //System.out.println("idl-file: " + args[i]);
            String path = args[i];
            File idlfile = new File(path);

            if (!idlfile.exists()) {
              System.out.println("Error: failed to open file: " + path);
              return false;
            }

            addFilesFromFile(idlfile, extraArgs);

          // find out if we have a project directory given
          } else if (args[i].equals("-P") && (i < args.length)) {
            if (projDirGiven) {
              System.out.println("Error: only one '-P' argument is allowed");
              return false;
            }
            projDirGiven = true;
            i++;
            // System.out.println("Project Directory: " + args[i]);
            String path = args[i];

            // Check that folders 'opsproject' and 'src' exist
            // and that 'opsproject' contains 'project.properties'
            File dir = new File(path);
            if (!dir.isDirectory()) {
              System.out.println("Error: argument is not a directory: " + path);
              return false;
            }
            try {
              path = dir.getCanonicalPath();
            } catch (java.io.IOException ioe) {
            }
            _strProjectDir = path;
            //System.out.println(">>>>: _strProjectDir= " + _strProjectDir);

            File projDir = new File(path + File.separator + "opsproject");
            File srcDir = new File(path + File.separator + "src");
            if ( (!projDir.isDirectory()) || (!srcDir.isDirectory()) ) {
              System.out.println("Error: missing 'opsproject' and/or 'src' directories in: " + path);
              return false;
            }
            File projProp = new File(path + File.separator + "opsproject" + File.separator + "project.properties");
            if (!projProp.exists()) {
              System.out.println("Error: missing file 'project.properties' in directory 'opsproject' in: " + path);
              return false;
            }

            // if so, extract projectname from directory name,
            try {
              File proj = dir.getCanonicalFile();
              extraArgs.add("-p");
              extraArgs.add(proj.getName());
            } catch (java.io.IOException ioe) {
            }

            // and set outputPath to 'Generated',
            extraArgs.add("-o");
            extraArgs.add(path + File.separator + "Generated");

            // and the project properties,
            extraArgs.add("-pp");
            extraArgs.add(path + File.separator + "opsproject" + File.separator + "project.properties");

            // and recursively add all idl-files starting with directory 'src'.
            addFilesFromDirectory(srcDir, extraArgs);

          } else {
            // move arguments to extraArgs vector
            extraArgs.add(args[i]);
          }
        }

        // then find out if we have a project property spec
        for(int i = 0 ; i < extraArgs.size() ; i++) {
          String arg = extraArgs.elementAt(i);
          if (arg.equals("-pp") && i < extraArgs.size()) {
            i++;
            arg = extraArgs.elementAt(i);
            System.out.println("Info: Parsing project properties in " + arg);

            // read file
            try {
              java.io.FileInputStream fis = new java.io.FileInputStream(arg);
              configlib.XMLArchiverIn xml = new configlib.XMLArchiverIn(fis, "properties");
              xml.add(ProjectProperties.getSerializableFactory());

              _props.serialize(xml);

            } catch (java.lang.NullPointerException npe) {
              System.out.println("configlib is buggy!");
            } catch (java.io.FileNotFoundException fnfe) {
              System.out.println("no such file: " + arg);
            } catch(java.io.IOException ioe) {
              System.out.println(arg + " can't be read from");
            } catch (configlib.exception.FormatException fe) {
              System.out.println(arg + " is badly formatted");
            }
          }
        }

        boolean refIdl = false;

        // finally parse remaining arguments
        for(int i = 0 ; i < extraArgs.size() ; i++) {
            String arg = extraArgs.elementAt(i);
            if(arg.equals("-o") && i < extraArgs.size()) {
                i++;
                arg = extraArgs.elementAt(i);
                File tmp = new File(arg + "");
                _props.setProperty(new Property("outputPath", tmp.getAbsolutePath()));
                System.out.println("Info: Output path set to " + _props.getPropertyValue("outputPath", null));
            } else if(arg.equals("-t") && (i < extraArgs.size())) {
                i++;
                arg = extraArgs.elementAt(i);
                _props.setProperty(new Property("templatePath", arg));
                System.out.println("Info: Template path set to " + _props.getPropertyValue("templatePath", null));
            } else if(arg.equals("-p") && (i < extraArgs.size())) {
                i++;
                arg = extraArgs.elementAt(i);
                _strProjectName = arg;
                System.out.println("Info: Project name set to " + _strProjectName);
            } else if((arg.equals("-b") || arg.equals("-B")) && (i < extraArgs.size())) {
                i++;
                updateBuildProp(extraArgs.elementAt(i), arg.equals("-b"));
              //} else if(arg.equals("-d")) {
              // -d is handled in first step above
              //} else if(arg.equals("-dd")) {
              // -dd is handled in first step above
            } else if (arg.equals("-dll") && (i < extraArgs.size())) {
                i++;
                _props.csBuildDllDependencies.add(new JarDependency(extraArgs.elementAt(i)));
            } else if (arg.equals("-dump")) {
                _dumpFlag = true;
            } else if (arg.equals("-fac")) {
                _bOnlyGenFactories = true;
            } else if((arg.equals("-g") || arg.equals("-G")) && (i < extraArgs.size())) {
                i++;
                updateGenerateProp(extraArgs.elementAt(i), arg.equals("-g"));
            } else if (arg.equals("-gpr") && (i < extraArgs.size())) {
                i++;
                _strOps4GprPath = extraArgs.elementAt(i);
            //} else if(arg.equals("-idls")) {
            // -idls are handled in first step above
            } else if (arg.equals("-jar") && (i < extraArgs.size())) {
                i++;
                _props.javaBuildJarDependencies.add(new JarDependency(extraArgs.elementAt(i)));
            } else if(arg.equals("-parse")) {
                _bOnlyParse = true;
            } else if (arg.equals("-pypac") && (i < extraArgs.size())) {
                i++;
                _props.pythonPackageDependencies.add(new JarDependency(extraArgs.elementAt(i)));
            } else if(arg.equals("-?") || arg.equals("-h") || arg.equals("--help")) {
                usage();
                System.exit(1);
            } else if(arg.equals("-printProps")) {
                System.out.println("");
                System.getProperties().list(System.out);
                System.out.println("");
                _props.list(System.out);
                System.exit(1);
            } else if(arg.equals("-pp")) {
                // ignore -pp here
                i++;
            //} else if(arg.equals("-P")) {
            // -P is handled in first step above
            } else if((arg.equals("-s") || (arg.equals("-S"))) && (i < extraArgs.size())) {
                i++;
                String special = extraArgs.elementAt(i);
                if(special.equals("mempool")) _bGenMemoryPool = arg.equals("-s");
                if(special.equals("jsonver")) _bJsonVersion = arg.equals("-s");
                if(special.equals("pyinit")) _bPythonInit = arg.equals("-s");
                if(special.equals("pypack")) _bPythonPackage = arg.equals("-s");
            } else if(arg.equals("-ref")) {
                refIdl = true;
            } else if(arg.equals("-retain") && (i < extraArgs.size())) {
                i++;
                String retain = extraArgs.elementAt(i);
                if(retain.equals("char")) _bKeepCharForByte = true;
            } else {
                // not a known option - regard as input file
                tmpFiles.clear();
                if(arg.endsWith("/") || arg.endsWith("\\")) {
                    // directory, add all files recursively
                    File dir = new File(arg);
                    if (!dir.isDirectory()) {
                        System.out.println("Error: argument is not a directory: " + arg);
                        return false;
                    }
                    try {
                        arg = dir.getCanonicalPath();
                        System.out.println("Info: recursively using idl's in directory: " + arg);
                        addFilesFromDirectory(dir, tmpFiles);
                    } catch (java.io.IOException ioe) {
                    }
                } else {
                    tmpFiles.add(arg);
                }
                // Add file(s) if not already in list
                for (String name : tmpFiles) {
                    boolean found = false;
                    for(String input : _listInputFiles) {
                        if(input.equals(name)) found = true;
                    }
                    for(String input : _listRefFiles) {
                        if(input.equals(name)) found = true;
                    }
                    if (refIdl) {
                        if(!found) _listRefFiles.add(name);
                    } else {
                        if(!found) _listInputFiles.add(name);
                    }
                }
            }
        }
        return true;
    }

    /** Function that wraps loading a file and running it
     *  through the IDLParser. The _parser will hold all
     *  loaded classes. Nothing is stored here.
     *  @return true on success */
    protected boolean parseFile(java.io.File inputfile, boolean refClass) {
        String fileString;

        try {
            // read file
            java.io.FileInputStream fis = new java.io.FileInputStream(inputfile);
            byte[] b = new byte[fis.available()];
            fis.read(b);
            fileString = new String(b);
        } catch(java.io.FileNotFoundException fnfe) {
            System.out.println("Error: " + inputfile.getName() + " doesn't exist");
            return false;
        } catch(java.io.IOException ioe) {
            System.out.println("Error: " + inputfile.getName() + " can't be read from");
            return false;
        }

        // parse the file
        String filename_wo_ext = inputfile.getName().substring(0, inputfile.getName().lastIndexOf("."));
        parsing.IDLClass idlclass = _parser.parse(filename_wo_ext, fileString, refClass);

        return idlclass != null;
    }

    protected boolean compilePython() {
        // create the compiler and set parameters
        opsc.PythonCompiler compiler = new opsc.PythonCompiler(_strProjectName);
        compiler.setVerbose(_verbose);
        compiler.setGenPythonInit(_bPythonInit);
        compiler.setGenPythonPackage(_bPythonPackage);
        compiler.setPackageNamesInCompilation(_packageNamesInCompilation);
        compiler.setPythonPackageDependencies(_props.pythonPackageDependencies);
        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            compiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if(propOutPath != null)
            compiler.setOutputDir(propOutPath.value + File.separator + "Python");

        compiler.compileDataClasses(_parser._idlClasses, "baba");
        //compiler.compileTypeSupport();

        return true;
    }

    protected boolean compileCpp() {
        // create the compiler and set parameters
        opsc.CppCompiler compiler = new opsc.CppCompiler(_strProjectName);
        compiler.setVerbose(_verbose);
        compiler.setGenOnlyTypeSupport(_bOnlyGenFactories);
        compiler.setGenMemoryPool(_bGenMemoryPool);
        compiler.keepCharAsIdlByte = _bKeepCharForByte;
        Property propTemplatePath = _props.getProperty("templatePath");
        if (propTemplatePath != null) compiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if (propOutPath != null) compiler.setOutputDir(propOutPath.value + File.separator + "Cpp");

        compiler.compileDataClasses(_parser._idlClasses, "baba");
        //compiler.compileTypeSupport();

        return true;
    }

    protected boolean compileJava() {
        // create the compiler and set parameters
        _javaCompiler = new opsc.JavaCompiler(_strProjectName);
        _javaCompiler.setVerbose(_verbose);
        _javaCompiler.setGenOnlyTypeSupport(_bOnlyGenFactories);
        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            _javaCompiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if(propOutPath != null)
            _javaCompiler.setOutputDir(propOutPath.value + File.separator + "Java");

        _javaCompiler.compileDataClasses(_parser._idlClasses, "baba");
        //_javaCompiler.compileTypeSupport();

        return true;
    }

    protected boolean buildJava() {
        try {
            _javaCompiler.setJarDependencies(_props.javaBuildJarDependencies);
            if (_strProjectDir.equals("")) _strProjectDir = _strProjectName;
            _javaCompiler.buildAndJar(_strProjectDir);
        } catch(java.io.IOException ioe) {
            System.out.println("Error: Failed to buildAndJar " + _strProjectName);
        } catch(java.lang.InterruptedException inte) {
            System.out.println("Error: Failed to buildAndJar (Interrupted) " + _strProjectName);
        }

        return true;
    }

    protected boolean compileCs() {
        // create the compiler and set parameters
        _CSharpCompiler = new opsc.CSharpCompiler(_strProjectName);
        _CSharpCompiler.setVerbose(_verbose);
        _CSharpCompiler.setGenOnlyTypeSupport(_bOnlyGenFactories);
        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            _CSharpCompiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if(propOutPath != null)
            _CSharpCompiler.setOutputDir(propOutPath.value + File.separator + "CSharp");

        _CSharpCompiler.compileDataClasses(_parser._idlClasses, "baba");
        //_CSharpCompiler.compileTypeSupport();

        return true;
    }

    protected boolean buildCs() {
        try {
            _CSharpCompiler.setDllDependencies(_props.csBuildDllDependencies);
            if (_strProjectDir.equals("")) _strProjectDir = _strProjectName;
            _CSharpCompiler.buildDll(_strProjectDir);
        } catch(java.io.IOException ioe) {
            System.out.println("Error: Failed to buildDll " + _strProjectName);
        } catch(java.lang.InterruptedException inte) {
            System.out.println("Error: Failed to buildDll (Interrupted) " + _strProjectName);
        }
        return true;
    }

    protected boolean compileDelphi() {
        // create the compiler and set parameters
        _DelphiCompiler = new opsc.DelphiCompiler(_strProjectName);
        _DelphiCompiler.setVerbose(_verbose);
        _DelphiCompiler.setGenOnlyTypeSupport(_bOnlyGenFactories);
        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            _DelphiCompiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if(propOutPath != null)
            _DelphiCompiler.setOutputDir(propOutPath.value + File.separator + "Delphi");

        _DelphiCompiler.compileDataClasses(_parser._idlClasses, "baba");
        //_DelphiCompiler.compileTypeSupport();

        return true;
    }

    protected boolean compileAda() {
        // create the compiler and set parameters
        _AdaCompiler = new opsc.AdaCompiler(_strProjectName);
        _AdaCompiler.setVerbose(_verbose);
        _AdaCompiler.setGenOnlyTypeSupport(_bOnlyGenFactories);
        _AdaCompiler.ops4GprPath = _strOps4GprPath;
        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            _AdaCompiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if(propOutPath != null)
            _AdaCompiler.setOutputDir(propOutPath.value + File.separator + "Ada");

        _AdaCompiler.compileDataClasses(_parser._idlClasses, "baba");
        //_AdaCompiler.compileTypeSupport();

        return true;
    }

    protected boolean compileJSON() {
        // create the compiler and set parameters
        _JSONCompiler = new opsc.JSONCompiler(_strProjectName);
        _JSONCompiler.setVerbose(_verbose);
        _JSONCompiler.includeVersion = _bJsonVersion;
        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            _JSONCompiler.setTemplateDir(propTemplatePath.value);
        Property propOutPath = _props.getProperty("outputPath");
        if(propOutPath != null)
            _JSONCompiler.setOutputDir(propOutPath.value + File.separator + "JSON");

        _JSONCompiler.compileDataClasses(_parser._idlClasses, "baba");

        return true;
    }

    protected boolean buildDebugProject() {

        opsc.DebugProjectCompiler compiler = new opsc.DebugProjectCompiler(_strProjectName);

        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            compiler.setTemplateDir(propTemplatePath.value);

        Property propOutPath = _props.getProperty("outputPath");

        compiler.createDebugProjectFile(propOutPath.value, _strProjectName, _props);

        return true;
    }

    protected boolean buildVSExample() {

        opsc.VisualStudio2008CppExampleCompiler compiler = new opsc.VisualStudio2008CppExampleCompiler(_strProjectName);

        Property propTemplatePath = _props.getProperty("templatePath");
        if(propTemplatePath != null)
            compiler.setTemplateDir(propTemplatePath.value);

        Property propOutPath = _props.getProperty("outputPath");

        compiler.compileVSCppExample(propOutPath.value, _strProjectName, _props);

        return true;
    }

    // ------------------------------------------------------------------------

    // Called for idlTypes to check if we can narrow them to more specific types
    protected boolean lookupType(Vector<IDLClass> list, IDLClass cl, IDLField field)
    {
      String fieldTypeName = field.getType().replace("[]","");
      String pName = "", cName = "", tName = "";
      int idx = fieldTypeName.lastIndexOf('.');
      if (idx > 0) {
        cName = fieldTypeName.substring(0, idx);
        tName = fieldTypeName.substring(idx+1);
      }
      if (cName.equals("")) return false;
      idx = cName.lastIndexOf('.');
      if (idx > 0) {
        pName = cName.substring(0, idx);
        cName = cName.substring(idx+1);
      }
      //System.out.println(">>>> lookupType(), pName: " + pName + " cName: " + cName + " tName: " + tName);
      for (IDLClass idlClass : list) {
        if (idlClass.getClassName().equals(cName)) {
          for (IDLEnumType et : idlClass.getEnumTypes()) {
            if (et.getName().equals(tName)) {
              // Change this type to an enum from another idl and define the init value
              field.setIdlType(false);
              field.setEnumType(true);
              field.setValue(et.getEnumNames().get(0));
              field.setRange("0", Integer.toString(et.getEnumNames().size()-1));
              if (pName.equals("")) {
                field.setFullyQualifiedType(idlClass.getPackageName() + "." + field.getType());
              }
              // Add an import so that we include the class containing the enum definition
              if (pName.equals("") || (pName.equals(cl.getPackageName()))) {
                cl.addImport(cName);
              } else {
                cl.addImport(pName + "." + cName);
              }
              return true;
            }
          }
        }
      }
      return false;
    }

    protected void secondStage()
    {
      // Connect class to base class
      _parser.connectClasses();

      for (IDLClass idlClass : _parser._idlClasses) {
        //
        _packageNamesInCompilation.add(idlClass.getPackageName());

        // Fix some things that the parser couldn't
        int version = -1;
        for (IDLField field : idlClass.getFields()) {
          // Parser only set enumType for our locally defined enums
          // Set first enum value as init value for enum fields
          if (field.isEnumType()) {
            // look up type in local scope
            for (IDLEnumType et : idlClass.getEnumTypes()) {
              if (et.getName().equals(field.getType().replace("[]",""))) {
                field.setValue(et.getEnumNames().get(0));
                field.setRange("0", Integer.toString(et.getEnumNames().size()-1));
                field.setFullyQualifiedType(idlClass.getPackageName() + "." + idlClass.getClassName() + "." + field.getType());
                break;
              }
            }
          }
          // Check if any idl types actually are enum types from other idls
          // ex: Definitions.Command
          if (field.isIdlType()) {
            // First look in the 'normal' classes and if not found in the 'ref' classes
            if (lookupType(_parser._idlClasses, idlClass, field) == false) {
              boolean dummy = lookupType(_parser._idlRefClasses, idlClass, field);
            }
          }
          // Check if any field specified the version directive and if so save the highest version found
          int v = CompilerSupport.highestVersion(idlClass.getClassName() + "." + field.getName(), field.getDirective());
          if (version < v) { version = v; }

          // Populate the field with values from directives, if any
          CompilerSupport.rangeDirective("", field);
          CompilerSupport.maxDirective("", field);
        }
        idlClass.setVersion(version);
      }

      Iterator<String> it = _packageNamesInCompilation.iterator();
      System.out.print("Info: IDL packages:");
      while(it.hasNext()) {
        System.out.print(" " + it.next());
      }
      System.out.println("");

    }

    // ------------------------------------------------------------------------

    protected void dump()
    {
      System.out.println("");
      for (IDLClass idlClass : _parser._idlClasses) {
        System.out.println("idlClass.getPackageName()   : " + idlClass.getPackageName());
        System.out.println("idlClass.getClassName()     : " + idlClass.getClassName());
        System.out.println("idlClass.getBaseClassName() : " + idlClass.getBaseClassName());
        System.out.println("idlClass.getDirective()     : " + idlClass.getDirective());
        System.out.println("idlClass.getVersion()       : " + Integer.toString(idlClass.getVersion()));
        System.out.println("idlClass.getComment()       : " + idlClass.getComment());

        IDLClass bc = idlClass.getBaseClassRef();
        if (bc != null) {
          System.out.println("idlClass.getBaseClassRef()  : " + bc.getClassName());
        }

        if (idlClass.getType() == IDLClass.ENUM_TYPE) {
          for (String str : idlClass.getEnumNames()) {
            System.out.println("  enum : " + str);
          }
          System.out.println("");
        } else {
          for (IDLEnumType et : idlClass.getEnumTypes()) {
            System.out.println("  et.getName()        : " + et.getName());
            System.out.println("    et.getComment()   : " + et.getComment());
            System.out.println("    et.getEnumNames() : ");
            for (String val : et.getEnumNames()) {
              System.out.println("         : " + val);
            }
            System.out.println("");
          }
          for (IDLField field : idlClass.getFields()) {
            System.out.println("  field.getName()        : " + field.getName());
            System.out.println("    field.getArraySize()          : " + field.getArraySize());
            System.out.println("    field.getArrayMaxSize()       : " + field.getArrayMaxSize());
            System.out.println("    field.getStringSize()         : " + field.getStringSize());
            System.out.println("    field.getStringMaxSize()      : " + field.getStringMaxSize());
            System.out.println("    field.getType()               : " + field.getType());
            System.out.println("    field.getFullyQualifiedType() : " + field.getFullyQualifiedType());
            System.out.println("    field.getComment()            : " + field.getComment());
            System.out.println("    field.getDirective()          : " + field.getDirective());
            System.out.println("    field.getRange()              : " + field.getRangeLo() + ".." + field.getRangeHi());
            System.out.println("    field.getValue()              : " + field.getValue());
            System.out.println("    field.isIdlType()             : " + field.isIdlType());
            System.out.println("    field.isEnumType()            : " + field.isEnumType());
            System.out.println("    field.isIntType()             : " + field.isIntType());
            System.out.println("    field.isFloatType()           : " + field.isFloatType());
            System.out.println("    field.isStringType()          : " + field.isStringType());
            System.out.println("    field.isArray()               : " + field.isArray());
            System.out.println("    field.isStatic()              : " + field.isStatic());
            System.out.println("    field.isAbstract()            : " + field.isAbstract());
            System.out.println("");
          }
        }
      }
    }

    protected void parseFileList(List<String> inputFiles, boolean refClass)
    {
        for(String input : inputFiles) {
            // Debug output
            if(_verbose > 0) {
                System.out.println("Debug: input: " + input);
            }

            if(input.endsWith(".idl")) {
                java.io.File inputfile = new java.io.File(input);
                if (!parseFile(inputfile, refClass)) {
                    System.exit(4);
                }

            } else if(input.endsWith(".prj")) {
                System.out.println("Error: We dont support prj files");
                System.exit(5);

            } else {
                System.out.println("Error: " + input + " unknown input type");
                System.exit(6);
            }
        }
    }

    public static void main(String args[]) {

        long startMs = System.currentTimeMillis();
        long mainStartMs = startMs;

        // instantiate this class
        OpsCompiler opsc = new OpsCompiler();
        if(!opsc.parseCommandLineArgs(args)) {
            System.out.println("Error: Bad args in command line");
            // return with an exit code
            System.exit( 1 );
        }

        // Debug output
        if(opsc._verbose > 0) {
            System.out.println("Debug: command line args ("+ (args.length-1) + ")");
            for(int i = 0 ; i < args.length ; i++) {
                System.out.println("  arg" + i + ": " + args[i] );
            }
        }

        if(opsc._listInputFiles.isEmpty()) {
            System.out.println("Error: No input files. Run command with -h for help.");
            // return with an exit code
            System.exit( 2 );
        }

        // iterate over all input files (idls) on the cmd line
        opsc.parseFileList(opsc._listRefFiles, true);
        opsc.parseFileList(opsc._listInputFiles, false);
        opsc.secondStage();

        if (opsc._dumpFlag) opsc.dump();

        System.out.flush();

        if (opsc._parser.getNrErrors() > 0) {
            // return with an exit code
            System.exit( 3 );
        }

        ///TODO Check that all types, which are not prefixed with a 'name.',
        /// are core types or defined with the idl's we parsed.

        long stopMs = System.currentTimeMillis();
        long parseMs = stopMs - startMs;
        startMs = stopMs;

        // Quit if we only should parse
        if(opsc._bOnlyParse) return;

        // generate python if requested
        if(opsc._props.generatePython) {
            opsc.compilePython();
        }

        stopMs = System.currentTimeMillis();
        long pythonMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate C++ if requested
        if(opsc._props.generateCpp) {
            opsc.compileCpp();
        }

        stopMs = System.currentTimeMillis();
        long cppMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate java if so requested
        if(opsc._props.generateJava) {
            // if compile is successful and user opted to build java
            if(opsc.compileJava() && opsc._props.buildJava) {
                System.out.println("");
                opsc.buildJava();
            }
        }

        stopMs = System.currentTimeMillis();
        long javaMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate C# if so requested
        if(opsc._props.generateCS) {
            // if compile is successful and user opted to build C#
            if(opsc.compileCs() && opsc._props.buildCS) {
                System.out.println("");
                opsc.buildCs();
            }
        }

        stopMs = System.currentTimeMillis();
        long csharpMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate Delphi if so requested
        if(opsc._props.generateDelphi) {
            opsc.compileDelphi();
        }

        stopMs = System.currentTimeMillis();
        long delphiMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate Ada if so requested
        if(opsc._props.generateAda) {
            opsc.compileAda();
        }

        stopMs = System.currentTimeMillis();
        long adaMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate JSON if so requested
        if(opsc._props.generateJSON) {
            opsc.compileJSON();
        }

        stopMs = System.currentTimeMillis();
        long jsonMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate debug project
        if(opsc._props.buildDebugProject) {
            System.out.println("");
            opsc.buildDebugProject();
        }

        stopMs = System.currentTimeMillis();
        long debugMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        // generate VS Example
        if(Boolean.parseBoolean(opsc._props.getPropertyValue("vsExampleEnabled", "false"))) {
            System.out.println("");
            opsc.buildVSExample();
        }

        stopMs = System.currentTimeMillis();
        long vsexampleMs = stopMs - startMs;
        startMs = stopMs;

        System.out.flush();

        long mainEndMs = startMs;

        System.out.println("");
        System.out.println("Info: Exec times [ms], " +
            " Parsing: " + parseMs +
            ", Ada: " + adaMs +
            ", C++: " + cppMs +
            ", C#: " + csharpMs +
            ", Debug: " + debugMs +
            ", Delphi: " + delphiMs +
            ", Java: " + javaMs +
            ", JSON: " + jsonMs +
            ", Python: " + pythonMs +
            ", VSExample: " + vsexampleMs +
            ", Total: " + (mainEndMs - mainStartMs)
        );

        System.out.flush();

    }
};
