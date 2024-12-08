# 311GIS v1.0

# 环境配置

软件：Visual Studio 2022 + Qt 6.7.2

第三方库：GDAL 3.9.0 + PROJ（GDAL 3.9.0 自带） + CGAL 5.6.1 + Boost 1.85.0

系统：Window 11

# 配置过程

## Visual Studio 2022

下载地址：<[Visual Studio: 面向软件开发人员和 Teams 的 IDE 和代码编辑器 (microsoft.com)](https://visualstudio.microsoft.com/zh-hans/)>

Qt Visual Studio Tools 下载地址：<[Qt Visual Studio Tools - Visual Studio Marketplace](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2022)>

## Qt 6,7,2

下载地址：<[Try Qt | Develop Applications and Embedded Systems | Qt](https://www.qt.io/download-dev)>



 **下载完Visual Studio 2022、Qt Visual Studio Tools和Qt 6.7.2后，在Visual Studio 2022的扩展中配置Qt 6.7.2的版本即可**

**详细配置过程可参考教程：**<[【VS2019安装+QT配置】_vs2019 配置qt-CSDN博客](https://blog.csdn.net/vor234/article/details/140625462?ops_request_misc=&request_id=&biz_id=102&utm_term=vs%E7%9A%84qt%E9%85%8D%E7%BD%AE&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-5-140625462.142^v100^pc_search_result_base6&spm=1018.2226.3001.4187)>



## GDAL 3.9.0 + PROJ

下载地址：<[Download — GDAL documentation](https://gdal.org/download.html)>



**下载完GDAL 3.9.0后，在Visual Studio 2022中的项目属性中配置include和lib等即可**

**详细配置过程可参考教程：**<[VS中配置GDAL库_vscode怎么连接gdal-CSDN博客](https://blog.csdn.net/u013230291/article/details/54231929?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522172378982616800185852754%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=172378982616800185852754&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-54231929-null-null.142^v100^pc_search_result_base6&utm_term=vs%E6%B7%BB%E5%8A%A0gdal%E5%BA%93&spm=1018.2226.3001.4187)>



## CGAL 5.6.1

下载地址：<[Download (cgal.org)](https://www.cgal.org/download.html)>



## Boost 1.85.0

下载地址：<[Boost C++ Libraries - Browse /boost-binaries at SourceForge.net](https://sourceforge.net/projects/boost/files/boost-binaries/)>



**Boost是CGAL的强制依赖项，想使用CGAL就必须配置好Boost**

**在下载完CGAL 5.6.1和Boost 1.85.0后，在Visual Studio 2022中的项目属性中配置include和lib等即可**

**详细配置过程可参考教程:**<[CGAL的安装与在VS中的配置_vs2022 c++添加cgal库-CSDN博客](https://blog.csdn.net/qq_35662333/article/details/138301989?ops_request_misc=&request_id=&biz_id=102&utm_term=cgal&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-2-138301989.142^v100^pc_search_result_base5&spm=1018.2226.3001.4187)>



**将以上软件及第三方库配置成功后，即可在Windows环境下运行311GIS软件**



# 开发人员

指导老师：

姚尧 中国地质大学（武汉）



用户界面开发人员：

向庆澳 中国地质大学（武汉）



算法设计人员：

向庆澳 中国地质大学（武汉）

李明宇 中国地质大学（武汉）

熊翰林 中国地质大学（武汉）

武佳豪 中国地质大学（武汉）


