package org.vaultdb.util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.BitSet;
import java.util.List;
import java.util.stream.Collectors;

import org.apache.commons.io.FilenameUtils;

public class FileUtilities {
	public static String readSQL(String filename) throws IOException {
		BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(filename)));
	    String sql = "";
	    String line;
	    while ((line = br.readLine()) != null) {
	    	
	    	line = line.replaceAll("\\-\\-.*$", ""); // delete any comments
	        sql = sql + line + " ";
	    }
	    br.close();
	    sql = sql.replaceAll("`", "");

	    return sql;
	}

	public static List<String> readFile(String filename) throws IOException  {	
		List<String> lines = null;
	
	if(System.getProperty("vaultdb.root") != null) {
			InputStream is = Utilities.class.getClassLoader().getResourceAsStream(filename);
			lines = new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8)).lines().collect(Collectors.toList());
	} else {
			lines = Files.readAllLines(Paths.get(filename), StandardCharsets.UTF_8);
	}
	
		return lines;				
	}

	
	public static void writeFile(String fname, String contents) throws FileNotFoundException, UnsupportedEncodingException {
	     String path = FilenameUtils.getFullPath(fname);
	     File f = new File(path);
	     f.mkdirs();
	
	     PrintWriter writer = new PrintWriter(fname, "UTF-8");
	     writer.write(contents);
	     writer.close();
	
	
	 }

	public static void copyFile(String src, String dst) throws Exception {
		String cmd = "cp " + src + " " + dst;
		
		String cwd = System.getProperty("user.dir");
    	CommandOutput out = Utilities.runCmd(cmd, cwd);
    	if(out.exitCode != 0) {
    		throw new Exception("File copy failed!");
    	}
    

	}
	
	public static byte[] readGeneratedClassFile(String packageName) throws IOException {
		String filename = Utilities.getCodeGenTarget() + "/"  + packageName.replace('.', '/') + "/NoClass.class";
		return FileUtilities.readBinaryFile(filename);
	}

	public static byte[] readBinaryFile(String filename) throws IOException {
	 	  Path p = FileSystems.getDefault().getPath("", filename);
	 	  return Files.readAllBytes(p);	 
	}
	
	// works for directories too
	public static boolean fileExists(String file) {
		File tmp = new File(file);
		return tmp.exists();
	}
	
	public static byte[] readByteFile(String filename) throws IOException {
		Path path = Paths.get(filename);
	    return Files.readAllBytes(path);
	}
	
	
	public static BitSet readBoolFile(String filename) throws IOException {
		byte[] bytes = readByteFile(filename);
		return BitSet.valueOf(bytes);
	    
	    
	}
}
