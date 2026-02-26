#pragma once

#include "../Engine/Classes/IO.h"
#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Classes/Helpers.h"

#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/regex.hpp>

namespace GX
{
	class SolutionWorker
	{
	private:
		std::string solutionText =
			"Microsoft Visual Studio Solution File, Format Version 12.00\n"
			"# Visual Studio 15\n"
			"VisualStudioVersion = 15.0.27703.2042\n"
			"MinimumVisualStudioVersion = 10.0.40219.1\n"
			"Project(\"{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}\") = \"%ProjectName%\", \"%ProjectName%.csproj\", \"{%GUID%}\"\n"
			"EndProject\n"
			"Global\n"
			"GlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
			"Debug | Any CPU = Debug | Any CPU\n"
			"Release | Any CPU = Release | Any CPU\n"
			"EndGlobalSection\n"
			"GlobalSection(ProjectConfigurationPlatforms) = postSolution\n"
			"{%GUID%}.Debug | Any CPU.ActiveCfg = Debug | Any CPU\n"
			"{%GUID%}.Debug | Any CPU.Build.0 = Debug | Any CPU\n"
			"{%GUID%}.Release | Any CPU.ActiveCfg = Release | Any CPU\n"
			"{%GUID%}.Release | Any CPU.Build.0 = Release | Any CPU\n"
			"EndGlobalSection\n"
			"GlobalSection(SolutionProperties) = preSolution\n"
			"HideSolutionNode = FALSE\n"
			"EndGlobalSection\n"
			"GlobalSection(ExtensibilityGlobals) = postSolution\n"
			"SolutionGuid = {1598A3D9-ED1B-4C73-AB0B-1D688AE3D601}\n"
			"EndGlobalSection\n"
			"EndGlobal";

		std::string projectText =
			"<?xml version = \"1.0\" encoding = \"utf-8\" ?>\n"
			"<Project ToolsVersion = \"15.0\" xmlns = \"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
			"	<Import Project = \"$(MSBuildExtensionsPath)\$(MSBuildToolsVersion)\Microsoft.Common.props\" Condition = \"Exists('$(MSBuildExtensionsPath)\$(MSBuildToolsVersion)\Microsoft.Common.props')\" />\n"
			"	<PropertyGroup>\n"
			"		<Configuration Condition = \" '$(Configuration)' == '' \">Debug</Configuration>\n"
			"		<Platform Condition = \" '$(Platform)' == '' \">AnyCPU</Platform>\n"
			"		<ProjectGuid>{%GUID%}</ProjectGuid>\n"
			"		<OutputType>Library</OutputType>\n"
			"		<AppDesignerFolder>Properties</AppDesignerFolder>\n"
			"		<RootNamespace>%ProjectName%</RootNamespace>\n"
			"		<AssemblyName>%ProjectName%</AssemblyName>\n"
			"		<TargetFrameworkVersion>v4.5.1</TargetFrameworkVersion>\n"
			"		<FileAlignment>512</FileAlignment>\n"
			"	</PropertyGroup>\n"
			"	<PropertyGroup Condition = \" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' \">\n"
			"		<DebugSymbols>true</DebugSymbols>\n"
			"		<DebugType>full</DebugType>\n"
			"		<Optimize>false</Optimize>\n"
			"		<OutputPath>Project\\bin\\Debug\\</OutputPath>\n"
			"		<BaseIntermediateOutputPath>Project\\obj\\</BaseIntermediateOutputPath>\n"
			"		<DefineConstants>DEBUG; TRACE</DefineConstants>\n"
			"		<ErrorReport>prompt</ErrorReport>\n"
			"		<WarningLevel>4</WarningLevel>\n"
			"	</PropertyGroup>\n"
			"	<PropertyGroup Condition = \" '$(Configuration)|$(Platform)' == 'Release|AnyCPU'\">\n"
			"		<DebugType>pdbonly</DebugType>\n"
			"		<Optimize>true</Optimize>\n"
			"		<OutputPath>Project\\bin\\Release\\</OutputPath>\n"
			"		<BaseIntermediateOutputPath>Project\\obj\\</BaseIntermediateOutputPath>\n"
			"		<DefineConstants>TRACE</DefineConstants>\n"
			"		<ErrorReport>prompt</ErrorReport>\n"
			"		<WarningLevel>4</WarningLevel>\n"
			"	</PropertyGroup>\n"
			"	<ItemGroup>\n"
			"		<Reference Include = \"FalcoEngine\">\n"
			"			<HintPath>%FalcoEngineDll%</HintPath>\n"
			"		</Reference>\n"
			"%ExternalDlls%\n"
			"		<Reference Include = \"System\" />\n"
			"		<Reference Include = \"System.Core\" />\n"
			"		<Reference Include = \"System.Xml.Linq\" />\n"
			"		<Reference Include = \"System.Data.DataSetExtensions\" />\n"
			"		<Reference Include = \"Microsoft.CSharp\" />\n"
			"		<Reference Include = \"System.Data\" />\n"
			"		<Reference Include = \"System.Net.Http\" />\n"
			"		<Reference Include = \"System.Xml\" />\n"
			"	</ItemGroup>\n"
			"	<ItemGroup>\n"
			"%SourceFiles%"
			"	</ItemGroup>\n"
			"	<Import Project = \"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />\n"
			"</Project>";

	public:
		void CreateSolution(std::string dir, std::string projectName, std::vector<std::string> files = {});
	};
}