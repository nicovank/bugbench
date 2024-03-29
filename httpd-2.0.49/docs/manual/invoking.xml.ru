<?xml version='1.0' encoding='KOI8-R' ?>
<!DOCTYPE manualpage SYSTEM "./style/manualpage.dtd">

<!--
 Copyright 2003-2004 The Apache Software Foundation

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->

<!--***************************************************-->
<!-- English revision: 1.2.2.3                         -->
<!--***************************************************-->
<!-- Translator: Ilia Soldis (rkai@tversu.ru)          -->
<!-- Reviewers:                                        -->
<!--             Ivan Shvedov (ivan@tversu.ru)         -->
<!--             Arthur Reznikov (art@altair.tversu.ru)-->
<!--***************************************************-->

<?xml-stylesheet type="text/xsl" href="./style/manual.ru.xsl"?>
<manualpage metafile="invoking.xml.meta">

  <title>������ Apache</title>

<summary>
    <p>�� Windows ���������� Apache ������ �������� ��� ������ Windows NT/2000/XP
    ��� ��� ���������� ���������� Windows 95/ME. ��� ��������� ����� ���������
    ���������� �� ����� �������, ���������� � ����������, ����������� <a
    href="platform/windows.html#winsvc">������ Apache ��� Windows � �������� �������</a> �
    <a href="platform/windows.xml#wincons">������ Apache ��� Windows � �������� �����������
    ����������</a>.</p>

    <p>� Unix ��������� <a href="programs/httpd.html">httpd</a> ������������ �����
    ������, �������������� � ������� ������ � �������������� ����������� �������.
    � ���, ����� ������� ����� ��������� <code>httpd</code> � ��� � ���������� �����
    ���������, � �������������� � ���� ���������.</p>
</summary>

<seealso><a href="stopping.html">������� � ����������</a></seealso>
<seealso><a href="programs/httpd.html">httpd</a></seealso>
<seealso><a href="programs/apachectl.html">apachectl</a></seealso>

<section id="startup"><title>��� ���������� � ������ ������� Apache</title>

    <p>���� � ��������� <directive module="mpm_common">Listen</directive>
    � ���������������� ����� ������� �������� 80 (���������� �� ���������)
    ��� ����� ������ �������� ����� ������� 1024, �� ��� ������� Apache,
    ���������� ���� ����������������� �������������, ��� ��� Apache ��������
    ������������ � ������������������ �����. ����� ����, ��� ������ ����������
    � �������� ��� ���������������� ��������, ����� ��� �������� ����� log - ������,
    �� ��������� ��������� <em>��������� ��������</em>, ������� � ����� ���������
    ��� ������ �� ��������� �������� �� ��������. �������� ������� <code>httpd</code>
    ����������� � ������� ������������������ ������������, � �� ����� ��� ��������
    ������� ����� ������� ���������. ��� ��� �������������� <a href="mpm.html">�� -
    �������</a>, ������� ������������� ������ � ��������.</p>

    <p>��� ������� ������ <code>httpd</code> ����� ����� ������������ ������
    <a href="programs/apachectl.html">apachectl</a>. ���� ������ ������������� ���������
    ���������� ���������, ����������� ��� ���������� ������ ������� ��� ����������
    ������������� ���������, � ����� ��������� ����������� ���� <code>httpd</code>.
    ������ <code>apachectl</code> �������� ������� ����� ��������� ������, ��� ���,
    ��� ������ ����� ��������� � ��� ��������� ������ ��� ����������� ��� ������� �����.
    �� ����� ������ ������� ������ ��������� ��������� � ������ <code>apachectl</code>,
    � ���������, ������� �������� ���������� <code>HTTPD</code> ��� ������� Apache
    �� ������� ��������, � ������ �����, ������� ����� ������������ ������� <em>������ ���</em>
    ��� ��� �������.</p>

    <p>������ ����� <code>httpd</code> ������� � ��������� <a href="configuring.html">����������������
    ����</a> <code>httpd.conf</code>. ���� � ����� ����� �������� ��� �� ����� ������ �������,
    �� ��� ����� �������� � ����� �����, �������� ������ � ������ <code>-f</code>, ��� ��� ��������
    � ��������� �������</p>

<example>/usr/local/apache2/bin/apachectl -f
      /usr/local/apache2/conf/httpd.conf</example>

    <p>���� �� ����� ������� �� �������� ������� �������, �� ������ ������������
    �� ������� � ����������� �� ���� ��������� ������ �������� � ������������
    ����������� ���������. ��� ��������� �� ��, ��� ������ ���������� � ������
    ��������� ���� ������. ������ �� ������, ��������� �������, ������������ �
    ���� � ������� �������� ��������, ����������� � ��������
    <directive module="core">DocumentRoot</directive>, � ����� ��������� ����� ������������,
    ������ �� ������� �� ������� �� ��� �� ��������.</p>
</section>

<section id="errors"><title>������, ������� ����� ���������� �� ����� �������</title>

    <p>���� �� ����� ������� Apache ���������� �����-���� ��������� ������,
    �� ����� ���, ��� ��������� ���� ������, ������ ������ �� ������� ��� �
    <directive module="core">ErrorLog</directive> ���������, �����������
    ������ ������. �������� ���������������� ���������� �� ������ ��������
    <code>"Unable to bind to Port ..."</code>. �������� ������ ��������� � ���� �������:</p>

    <ul>
      <li>���� �� ��������� ��������� ������ �� ����������������� �����, ������ ������������������
      � �������, ��� ������� ������������; ���</li>

      <li>���� �� ��������� ������� ������, ����� � ������� ��� ���� ������������� ����� Apache
      ��� ������ web-������, ��������� ��� �� ����� ����</li>
    </ul>

    <p>������� ������ �������� ������� ����� ����� �� ��������
    <a href="faq/">FAQ</a>.</p>
</section>

<section id="boot"><title>������ ������� ������ � �������� ���� �������</title>

    <p>���� �� ������, ����� ������ ���������� ������������� ����� ������������ �������,
    �������� ����� ������� <code>apachectl</code> � ��������� �����, ���������� �� ��������
    ������������ ����� (������ ��� <code>rc.local</code> ��� ����� � �������� <code>rc.N</code>).
    ��� �������� � ������� Apache �� ����� ������������������ ������������.
    �� ��������� ������� � ������������� �������, ���������, ��� ������ ��������������� ���������.</p>

    <p>������ <code>apachectl</code> ���������� ����� �������, ��� �� �����
    ����������� ��� ����������� init-������ ������� SysV; �� ����� ���������
    ��������� <code>start</code>, <code>restart</code>, � <code>stop</code>
    � ���������� �� � ��������������� ������� �������� <code>httpd</code>.
    ������� ���� ����� ��� ���������� ������� ������ �� <code>apachectl</code>
    � ��������� �������� �������� init. �� ������ ��� ������ ���, �������
    ������ ���������� ����� �������.</p>
</section>

<section id="info"><title>�������������� ����������</title>

    <p>�������������� ���������� �� ������ ��������� ������ <a href="programs/httpd.html">httpd</a>
    � <a href="programs/apachectl.html">apachectl</a>, � ����� ������
    ��������������� ��������, �� ������ ����� �� �������� <a href="programs/">"������
    � ��������������� ���������"</a>. ������� ����� <a href="mod/directives.html">������������</a>
    �� ��� ������, �������� � ����������� Apache, � ��� ���������, ������� ��� �������������.</p>
</section>

</manualpage>
