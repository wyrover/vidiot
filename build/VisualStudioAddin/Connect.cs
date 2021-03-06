using System;
using Extensibility;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.CommandBars;
using System.Resources;
using System.Reflection;
using System.Globalization;
using VSLangProj;
using System.Linq;
using System.Collections.Generic;
using System.Collections;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;

namespace VisualStudioAddin
{
    public class Connect : IDTExtensibility2, IDTCommandTarget, IVsSolutionEvents
    {

        #region Initialization

		public Connect()
		{
		}

		public void OnConnection(object application, ext_ConnectMode connectMode, object addInInst, ref Array custom)
		{
            try
            {
                _applicationObject = (DTE2)application;
                _addInInstance = (AddIn)addInInst;

                addCommand(sMain,       "Global::ALT+M");
                addCommand(sSwitch,     "Global::ALT+O");
                addCommand(sActivate,   "Global::ALT+T");
                addCommand(sSort,       "Global::ALT+S");

                solution = ServiceProvider.GlobalProvider.GetService(typeof(SVsSolution)) as IVsSolution2;
                if (solution != null)
                {
                    // Register for solution events
                    solution.AdviseSolutionEvents(this, out solutionEventsCookie);
                }
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show(e.Message);
            }
        }

        #endregion

        #region IDTExtensibility2

		public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
		{
		}

		public void OnAddInsUpdate(ref Array custom)
		{
		}

		public void OnStartupComplete(ref Array custom)
		{
		}

		public void OnBeginShutdown(ref Array custom)
		{
		}

        public void QueryStatus(string commandName, vsCommandStatusTextWanted neededText, ref vsCommandStatus status, ref object commandText)
        {
            if (neededText == vsCommandStatusTextWanted.vsCommandStatusTextWantedNone)
            {
                if ((commandName == sCommandNameSwitch) || 
                    (commandName == sCommandNameActivate) ||
                    (commandName == sCommandNameMain) ||
                    (commandName == sCommandNameSort))
                {
                    status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported | vsCommandStatus.vsCommandStatusEnabled;
                    return;
                }
            }
        }

        public void Exec(string commandName, vsCommandExecOption executeOption, ref object varIn, ref object varOut, ref bool handled)
        {
            try
            {
                if (executeOption == vsCommandExecOption.vsCommandExecOptionDoDefault)
                {
                    if (commandName == sCommandNameSwitch)
                    {
                        ExecuteSourceHeaderSwitch();
                    }
                    else if (commandName == sCommandNameActivate)
                    {
                        ExecuteActivateTest();
                    }
                    else if (commandName == sCommandNameMain)
                    {
                        ExecuteActivateMain();
                    }
                    else if (commandName == sCommandNameSort)
                    {
                        ExecuteSortSelectedLines();
                    }
                }
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show(e.Message);
            }
        }

        #endregion

        #region IVsSolutionEvents Members

        public int OnAfterCloseSolution(object pUnkReserved)
        {
            currentStartupProject = sentinel;
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnAfterLoadProject(IVsHierarchy pStubHierarchy, IVsHierarchy pRealHierarchy)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnAfterOpenProject(IVsHierarchy pHierarchy, int fAdded)
        {
            RegisterProject(pHierarchy);
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }
        
        public int OnAfterOpenSolution(object pUnkReserved, int fNewSolution)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnBeforeCloseProject(IVsHierarchy pHierarchy, int fRemoved)
        {
            try
            {
                object propNameObj = null;
                if (pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_Name, out propNameObj) == Microsoft.VisualStudio.VSConstants.S_OK)
                {
                    string name = (string)propNameObj;
                    if (currentStartupProject == name)
                    {
                        currentStartupProject = sentinel;
                    }
                    UnregisterProject(pHierarchy);
                }
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show("OnBeforeCloseProject: " + e.Message);
            } 
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnBeforeCloseSolution(object pUnkReserved)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnBeforeUnloadProject(IVsHierarchy pRealHierarchy, IVsHierarchy pStubHierarchy)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnQueryCloseProject(IVsHierarchy pHierarchy, int fRemoving, ref int pfCancel)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnQueryCloseSolution(object pUnkReserved, ref int pfCancel)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnQueryUnloadProject(IVsHierarchy pRealHierarchy, ref int pfCancel)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        #endregion

        #region IVsSolutionLoadEvents Members

        public int OnAfterBackgroundSolutionLoadComplete()
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnAfterLoadProjectBatch(bool fIsBackgroundIdleBatch)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnBeforeBackgroundSolutionLoadBegins()
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnBeforeLoadProjectBatch(bool fIsBackgroundIdleBatch)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnBeforeOpenSolution(string pszSolutionFilename)
        {
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        public int OnQueryBackgroundLoadProjectBatch(out bool pfShouldDelayLoadToNextIdle)
        {
            pfShouldDelayLoadToNextIdle = false;
            return Microsoft.VisualStudio.VSConstants.S_OK;
        }

        #endregion

        #region Execute commands methods

        private void TryOpenFile(string filename)
        {
            try
            {
                _applicationObject.ItemOperations.OpenFile(filename, EnvDTE.Constants.vsViewKindCode);
            }
            catch (Exception)
            {
                // Will happen since one of the documents (include vs. interface) is always non-existent.
                return;
            }

        }

        private void ExecuteSourceHeaderSwitch()
        {
            string docname = "";
            try
            {
                docname =  _applicationObject.ActiveDocument.FullName;
            }
            catch (Exception)
            {
                // Can happen in case no document open.
                return;
            }
            if (docname.EndsWith(".cpp", StringComparison.InvariantCultureIgnoreCase))
            {
                TryOpenFile(docname.Replace(".cpp", ".h").Replace("src", "include"));
                TryOpenFile(docname.Replace(".cpp", ".h").Replace("src", "interface"));
            }
            else if (docname.EndsWith(".h", StringComparison.InvariantCultureIgnoreCase))
            {
                TryOpenFile(docname.Replace(".h", ".cpp").Replace("include", "src"));
                TryOpenFile(docname.Replace(".h", ".cpp").Replace("interface", "src"));
            }
        }

        private void ExecuteActivateTest()
        {
            try
            {
                IVsSolutionBuildManager2 sbm2 = ServiceProvider.GlobalProvider.GetService(typeof(SVsSolutionBuildManager)) as IVsSolutionBuildManager2;
                // todo restore previous startup project upon startup?
                IVsHierarchy current;
                int i = sbm2.get_StartupProject(out current);
                String currentname = proj2name[current];
                int index = testProjects.FindIndex(delegate(String name) { return name.Equals(currentname); } );
                index++; // Note: FindIndex returns -1 in case of no match
                if (index >= testProjects.Count)
                {
                    index = 0;
                }
                sbm2.set_StartupProject(name2proj[testProjects[index]]);
            }
            catch (Exception)
            {
            }
        }

        private void ExecuteActivateMain()
        {
            try
            {
                IVsSolutionBuildManager2 sbm2 = ServiceProvider.GlobalProvider.GetService(typeof(SVsSolutionBuildManager)) as IVsSolutionBuildManager2;
                sbm2.set_StartupProject(name2proj["main"]);
            }
            catch (Exception)
            {
            }
        }

        private void ExecuteSortSelectedLines()
        {
            try
            {
                TextSelection selection = (TextSelection)_applicationObject.ActiveDocument.Selection;
                int beginline = selection.AnchorPoint.Line;
                int endline = selection.BottomPoint.Line;

                selection.GotoLine(beginline);
                selection.LineDown(true, endline - beginline);

                if (selection.Text.EndsWith(Environment.NewLine + Environment.NewLine))
                {
                    // Avoid taking the next empty line into account:
                    // xxx\r\n
                    // yyy\r\n
                    // <emptyline>
                    // Select the xxx, and yyy lines, then move the cursor one line down.
                    // The emptyline should not be taken into account when sorting.
                    selection.CharLeft(true, 1);
                }
                selection.CharLeft(true, 1);
                string[] lines = selection.Text.Split(new string[] { "\r\n", "\n" }, StringSplitOptions.None);
                Array.Sort(lines);
                _applicationObject.UndoContext.Open("Sort Lines");
                selection.Delete();
                selection.Insert(String.Join(Environment.NewLine, lines));
                _applicationObject.UndoContext.Close();
            }
            catch (Exception)
            {
            }
        }

        #endregion

        #region Helper methods

        private void RegisterProject(IVsHierarchy pHierarchy)
        {
            try
            {
                bool valid = true;
                object nameObj = null;
                object typeNameObj = null;
                object captionObj = null;
                valid &= pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_Name, out nameObj) == Microsoft.VisualStudio.VSConstants.S_OK;
                valid &= pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_TypeName, out typeNameObj) == Microsoft.VisualStudio.VSConstants.S_OK;
                valid &= pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_Caption, out captionObj) == Microsoft.VisualStudio.VSConstants.S_OK;
                if (valid)
                {
                    string name = (string)nameObj;
                    string typeName = (string)typeNameObj;
                    string caption = (string)captionObj;

                    name2proj.Add(name, pHierarchy);
                    proj2name.Add(pHierarchy, name);

                    if (name.ToLower().Contains("test"))
                    {
                        // todo add check for is executable project.
                        testProjects.Add(name);
                        testProjects.Sort();
                    }
                }
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show("RegisterProject: " + e.Message);
            }
        }

        private void UnregisterProject(IVsHierarchy pHierarchy)
        {
            try
            {
                bool valid = true;
                object nameObj = null;
                object typeNameObj = null;
                object captionObj = null;
                valid &= pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_Name, out nameObj) == Microsoft.VisualStudio.VSConstants.S_OK;
                valid &= pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_TypeName, out typeNameObj) == Microsoft.VisualStudio.VSConstants.S_OK;
                valid &= pHierarchy.GetProperty((uint)Microsoft.VisualStudio.VSConstants.VSITEMID.Root, (int)__VSHPROPID.VSHPROPID_Caption, out captionObj) == Microsoft.VisualStudio.VSConstants.S_OK;
                if (valid)
                {
                    string name = (string)nameObj;

                    name2proj.Remove(name);
                    proj2name.Remove(pHierarchy);

                    if (name.ToLower().Contains("test"))
                    {
                        testProjects.Remove(name);
                    }
                }
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show("UnregisterProject: " + e.Message);
            } 
        }

        private Project GetProject(IVsHierarchy pHierarchy)
        {
            object project;
            ErrorHandler.ThrowOnFailure
            (pHierarchy.GetProperty(Microsoft.VisualStudio.VSConstants.VSITEMID_ROOT,
                                                               (int)__VSHPROPID.VSHPROPID_ExtObject,
                                                               out project));
            return (project as Project);
        }

        private void addCommand(string name, string shortcut)
        {
            try
            {
                Commands2 commands = (Commands2)_applicationObject.Commands;
                object[] contextGUIDS = new object[] { };
                Command cmd = commands.AddNamedCommand2(_addInInstance, name, name, name, true, 2652, ref contextGUIDS, (int)vsCommandStatus.vsCommandStatusSupported + (int)vsCommandStatus.vsCommandStatusEnabled, (int)vsCommandStyle.vsCommandStylePictAndText, vsCommandControlType.vsCommandControlTypeButton);
                cmd.Bindings = shortcut;
            }
            catch (Exception)
            {
                // No error message is given, since the command may already have been registered.
                // System.Windows.Forms.MessageBox.Show(e.Message);
            }
        }
        #endregion

        #region Members

        private DTE2 _applicationObject;
        private AddIn _addInInstance;
        private const string sentinel = "";

        // Made static since somehow multiple instances of the addin are loaded....
        private static Dictionary<IVsHierarchy, string> proj2name = new Dictionary<IVsHierarchy, string>();
        private static Dictionary<string, IVsHierarchy> name2proj = new Dictionary<string, IVsHierarchy>();
        private static List<String> testProjects = new List<string>();

        private string currentStartupProject = sentinel;
        private IVsSolution2 solution = null;
        private uint solutionEventsCookie;

        const string sModuleName = "VisualStudioAddin";
        const string sClassName = "Connect";
        const string sSwitch = "SourceHeaderSwitcher";
        const string sActivate = "ActivateTest";
        const string sMain = "ActivateMain";
        const string sSort = "SortSelectedLines";
        const string sCommandNameSwitch = sModuleName + "." + sClassName + "." + sSwitch;
        const string sCommandNameActivate = sModuleName + "." + sClassName + "." + sActivate;
        const string sCommandNameSort = sModuleName + "." + sClassName + "." + sSort;
        const string sCommandNameMain = sModuleName + "." + sClassName + "." + sMain;

        #endregion
    }
}