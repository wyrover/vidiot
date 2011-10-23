using System;
using Extensibility;
using EnvDTE;
using EnvDTE80;
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

            // Register commands
            Commands2 commands = (Commands2)_applicationObject.Commands;
            object[] contextGUIDS = new object[] { };
            Command commandswitch = commands.AddNamedCommand2(_addInInstance, sSwitch, sSwitch, sSwitch, true, 2652, ref contextGUIDS, (int)vsCommandStatus.vsCommandStatusSupported + (int)vsCommandStatus.vsCommandStatusEnabled, (int)vsCommandStyle.vsCommandStylePictAndText, vsCommandControlType.vsCommandControlTypeButton);
            Command commandactivate = commands.AddNamedCommand2(_addInInstance, sActivate, sActivate, sActivate, true, 2652, ref contextGUIDS, (int)vsCommandStatus.vsCommandStatusSupported + (int)vsCommandStatus.vsCommandStatusEnabled, (int)vsCommandStyle.vsCommandStylePictAndText, vsCommandControlType.vsCommandControlTypeButton);


                commandswitch.Bindings = "Global::ALT+O";
                commandactivate.Bindings = "Global::ALT+T";

            solution = ServiceProvider.GlobalProvider.GetService(typeof(SVsSolution)) as IVsSolution2;
            if (solution != null)
            {
                // Register for solution events
                solution.AdviseSolutionEvents(this, out solutionEventsCookie);
            }
            }
            catch (Exception)
            {
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
                if ((commandName == sCommandNameSwitch) || (commandName == sCommandNameActivate))
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
                }
            }
            catch (Exception)
            {
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

        private void ExecuteSourceHeaderSwitch()
        {
            Document curdoc = _applicationObject.ActiveDocument;
            string docname = "";
            try
            {
                docname = curdoc.FullName;
            }
            catch (Exception)
            {
                // Can happen in case no document open.
                return;

            }
            if (docname.EndsWith(".cpp", StringComparison.InvariantCultureIgnoreCase))
            {
                _applicationObject.ItemOperations.OpenFile(docname.Replace(".cpp", ".h").Replace("src", "include"), EnvDTE.Constants.vsViewKindCode);
                _applicationObject.ItemOperations.OpenFile(docname.Replace(".cpp", ".h").Replace("src", "interface"), EnvDTE.Constants.vsViewKindCode);
            }
            else if (docname.EndsWith(".h", StringComparison.InvariantCultureIgnoreCase))
            {
                _applicationObject.ItemOperations.OpenFile(docname.Replace(".h", ".cpp").Replace("include", "src"), EnvDTE.Constants.vsViewKindCode);
                _applicationObject.ItemOperations.OpenFile(docname.Replace(".h", ".cpp").Replace("interface", "src"), EnvDTE.Constants.vsViewKindCode);
            }
        }

        private void ExecuteActivateTest()
        {
            try
            {
                IVsSolutionBuildManager2 sbm2 = ServiceProvider.GlobalProvider.GetService(typeof(SVsSolutionBuildManager)) as IVsSolutionBuildManager2;
                sbm2.set_StartupProject(name2proj["testauto"]);
            }
            catch (Exception)
            {
            }
        }

        #endregion

        #region Helper methods

        private void RegisterProject(IVsHierarchy pHierarchy)
        {
            // When project is opened: register it and its name
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

                Project project = GetProject(pHierarchy);
                var projectType = project.Properties.Item("ProjectType");
                var eProjectType = (VSLangProj.prjProjectType)projectType.Value;
                if (eProjectType == VSLangProj.prjProjectType.prjProjectTypeLocal)
                {
                    var outputType = project.Properties.Item("OutputType");
                    var eOutputType = (VSLangProj.prjOutputType)outputType.Value;
                    if (eOutputType == VSLangProj.prjOutputType.prjOutputTypeWinExe ||
                        eOutputType == VSLangProj.prjOutputType.prjOutputTypeExe)
                    {
                        executableProjects.Add(name);
                    }
                }
            }
        }

        private void UnregisterProject(IVsHierarchy pHierarchy)
        {
            String name = proj2name[pHierarchy];
            name2proj.Remove(name);
            proj2name.Remove(pHierarchy);
        }

        private Project GetProject(IVsHierarchy pHierarchy)
        {
            object project;
            pHierarchy.GetProperty(Microsoft.VisualStudio.VSConstants.VSITEMID_ROOT,
                                                               (int)__VSHPROPID.VSHPROPID_ExtObject,
                                                               out project);
            return (project as Project);
        }

        #endregion

        #region Members

        private DTE2 _applicationObject;
        private AddIn _addInInstance;
        private const string sentinel = "";

        // Made static since somehow multiple instances of the addin are loaded....
        private static Dictionary<IVsHierarchy, string> proj2name = new Dictionary<IVsHierarchy, string>();
        private static Dictionary<string, IVsHierarchy> name2proj = new Dictionary<string, IVsHierarchy>();
        private static List<string> executableProjects = new List<string>();

        private string currentStartupProject = sentinel;
        private IVsSolution2 solution = null;
        private uint solutionEventsCookie;

        const string sModuleName = "VisualStudioAddin";
        const string sClassName = "Connect";
        const string sSwitch = "SourceHeaderSwitcher";
        const string sActivate = "ActivateTest";
        const string sCommandNameSwitch = sModuleName + "." + sClassName + "." + sSwitch;
        const string sCommandNameActivate = sModuleName + "." + sClassName + "." + sActivate;

        #endregion
    }
}