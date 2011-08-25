Imports System
Imports EnvDTE
Imports EnvDTE80
Imports EnvDTE90
Imports EnvDTE90a
Imports EnvDTE100
Imports System.Diagnostics

Public Module CppUtilities
    '=====================================================================  
    ' If the currently open document is a CPP or an H file, attempts to  
    ' switch between the CPP and the H file.  
    '=====================================================================  
    Public Sub SwitchBetweenSourceAndHeader()
        Dim currentDocument As String
        Dim targetDocument As String

        currentDocument = ActiveDocument.FullName

        If currentDocument.EndsWith(".cpp", StringComparison.InvariantCultureIgnoreCase) Then
            targetDocument = Left(currentDocument, Len(currentDocument) - 3) + "h"
            targetDocument = targetDocument.Replace("src", "include")
            OpenDocument(targetDocument)
        ElseIf currentDocument.EndsWith(".h", StringComparison.InvariantCultureIgnoreCase) Then
            targetDocument = Left(currentDocument, Len(currentDocument) - 1) + "cpp"
            targetDocument = targetDocument.Replace("include", "src")
            OpenDocument(targetDocument)
        End If

    End Sub

    '=====================================================================  
    ' Given a document name, attempts to activate it if it is already open,  
    ' otherwise attempts to open it.  
    '=====================================================================  
    Private Sub OpenDocument(ByRef documentName As String)
        Dim document As EnvDTE.Document
        Dim activatedTarget As Boolean
        activatedTarget = False

        For Each document In Application.Documents
            If document.FullName = documentName And document.Windows.Count > 0 Then
                document.Activate()
                activatedTarget = True
                Exit For
            End If
        Next
        If Not activatedTarget Then
            If FileIO.FileSystem.FileExists(documentName) Then
                Application.Documents.Open(documentName, "Text")
            End If
        End If
    End Sub

    Sub SortSelectedText()
        Dim Selection As TextSelection = DTE.ActiveDocument.Selection
        If Selection.Text.EndsWith(Environment.NewLine) Then
            DTE.ActiveDocument.Selection.WordLeft(True)
        End If
        Dim Lines() As String = Selection.Text.Replace(Environment.NewLine, Chr(13)).Split(Chr(13))
        Array.Sort(Lines)
        DTE.UndoContext.Open("Sort Lines")
        Selection.Delete()
        Selection.Insert(String.Join(Environment.NewLine, Lines))
        DTE.UndoContext.Close()

    End Sub

    Sub ActivateTestManual()
        DTE.Windows.Item(Constants.vsWindowKindSolutionExplorer).Activate()
        DTE.ActiveWindow.Object.GetItem("Vidiot\testmanual").Select(vsUISelectionType.vsUISelectionTypeSelect)
        DTE.ExecuteCommand("Project.SetasStartUpProject")
    End Sub
    Sub ActivateTestAuto()
        DTE.Windows.Item(Constants.vsWindowKindSolutionExplorer).Activate()
        DTE.ActiveWindow.Object.GetItem("Vidiot\testauto").Select(vsUISelectionType.vsUISelectionTypeSelect)
        DTE.ExecuteCommand("Project.SetasStartUpProject")
    End Sub


End Module
