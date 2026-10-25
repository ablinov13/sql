# Генерирует DbLabs.sln и .vcxproj для всех лаб (запуск: python tools/gen_projects.py)
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
NS = uuid.UUID("6f1c2b44-1d2e-4c55-9a7e-0b5d1c3a9e10")
VC_TYPE = "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"


def guid(name):
    return str(uuid.uuid5(NS, name)).upper()


# имя проекта, тип, нужен ли sqlite
PROJECTS = [
    ("sqlite", "StaticLibrary", False),
    ("lab2", "Application", True),
    ("lab3", "Application", True),
    ("lab4", "Application", True),
    ("lab5_1", "Application", False),
    ("lab5_2", "Application", False),
    ("lab5_3", "Application", False),
]

CONFIGS = [("Debug", "true"), ("Release", "false")]


def vcxproj(name, kind, uses_sqlite):
    cfg_items = "\n".join(
        f'''    <ProjectConfiguration Include="{c}|x64">
      <Configuration>{c}</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>''' for c, _ in CONFIGS)
    cfg_groups = "\n".join(
        f'''  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='{c}|x64'" Label="Configuration">
    <ConfigurationType>{kind}</ConfigurationType>
    <UseDebugLibraries>{dbg}</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>{'' if dbg == 'true' else chr(10) + '    <WholeProgramOptimization>false</WholeProgramOptimization>'}
  </PropertyGroup>''' for c, dbg in CONFIGS)
    if name == "sqlite":
        sources = '''  <ItemGroup>
    <ClCompile Include="..\\third_party\\sqlite\\sqlite3.c">
      <CompileAs>CompileAsC</CompileAs>
      <WarningLevel>TurnOffAllWarnings</WarningLevel>
      <SDLCheck>false</SDLCheck>
    </ClCompile>
    <ClInclude Include="..\\third_party\\sqlite\\sqlite3.h" />
  </ItemGroup>'''
    else:
        sources = '''  <ItemGroup>
    <ClCompile Include="*.cpp" />
    <ClInclude Include="*.h" />
  </ItemGroup>'''
    ref = ""
    if uses_sqlite:
        ref = f'''
  <ItemGroup>
    <ProjectReference Include="..\\sqlite\\sqlite.vcxproj">
      <Project>{{{guid("sqlite")}}}</Project>
    </ProjectReference>
  </ItemGroup>'''
    return f'''<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
{cfg_items}
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{{{guid(name)}}}</ProjectGuid>
    <RootNamespace>{name}</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
{cfg_groups}
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <Import Project="..\\common.props" />
{sources}{ref}
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
</Project>
'''


def sln():
    lines = ["", "Microsoft Visual Studio Solution File, Format Version 12.00",
             "# Visual Studio Version 17", "VisualStudioVersion = 17.0.31903.59",
             "MinimumVisualStudioVersion = 10.0.40219.1"]
    for name, _, _ in PROJECTS:
        lines.append(f'Project("{{{VC_TYPE}}}") = "{name}", "{name}\\{name}.vcxproj", "{{{guid(name)}}}"')
        lines.append("EndProject")
    lines.append("Global")
    lines.append("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution")
    for c, _ in CONFIGS:
        lines.append(f"\t\t{c}|x64 = {c}|x64")
    lines.append("\tEndGlobalSection")
    lines.append("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution")
    for name, _, _ in PROJECTS:
        for c, _ in CONFIGS:
            lines.append(f"\t\t{{{guid(name)}}}.{c}|x64.ActiveCfg = {c}|x64")
            lines.append(f"\t\t{{{guid(name)}}}.{c}|x64.Build.0 = {c}|x64")
    lines.append("\tEndGlobalSection")
    lines.append("\tGlobalSection(SolutionProperties) = preSolution")
    lines.append("\t\tHideSolutionNode = FALSE")
    lines.append("\tEndGlobalSection")
    lines.append("EndGlobal")
    return "\r\n".join(lines) + "\r\n"


for name, kind, uses in PROJECTS:
    d = ROOT / name
    d.mkdir(exist_ok=True)
    (d / f"{name}.vcxproj").write_text(vcxproj(name, kind, uses), encoding="utf-8")
(ROOT / "DbLabs.sln").write_bytes(sln().encode("utf-8-sig"))
print("ok")
